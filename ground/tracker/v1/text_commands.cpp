
/*
 Copyright (c) 2013 Andy Little 

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with this program. If not, see <http://www.gnu.org/licenses/>
*/

#include <stm32f4xx.h>
#include <stdlib.h>
#include <stdio.h>

#include <quan/convert.hpp>
#include <quan/conversion/itoa.hpp>
#include <quan/conversion/float_convert.hpp>
#include "serial_ports.hpp"
#include "azimuth.hpp"
#include "telemetry.hpp"
#include "compass.hpp"
#include <quan/uav/position.hpp>
#include <quan/constrain.hpp>

/*
   "E"  --> enable azimuth motor
   "D" --> disable azimuth motor
   "$%f,%f,%f;"   --> set home position lat,lon,altitude
   "~%f,%f,%f;"    --> aircraft position  lat,lon,altitude
   "P%i" --> set azimuth position %i
   "Z" --> set zero (North)
   "kP%f" --> set proportional term %f
   "kD%f" --> set differential term %f
   "H%i" ---> set elev servo pos;

   "GT"  --> get target position
   "GA"  --> get actual position
   "Gp"  --> Get proportional term
   "Gd"  --> get differential term  
   "GM" --> get mag reading x,y,z    what this represents is dependendt on the last M+ M- M0 sent prior
   "Gm"  --> get raw mag reading what this represents is dependendt on the last M+ M- M0 sent prior
   "Gf" --> get compass offset // loaded from flash at startup
   "GB" --> get compass bearing. requires a valid mag offset
   "M+" --> Set Mag plus strap gain
   "M-" --> Set Mag - strap gain
   "M0" --> Set Mag - normal reading
   "Mf%f --> set raw mag filter value
// to do 
   "GE" --> get elevation servo pos;
    M!   --> request compass reading disabled (e.g want i2c bus for eeprom access)
    M?   --> is compass reading enabled ( disabling takes a bit to release bus transaction neds to complete)
    M*   --> enable compass reading
    Rd%s 
    Wr%s 
   
*/
namespace {

   typedef telemetry::gps_position uav_pos_type;

   bool parse_position(const char* cbuf, size_t len, uav_pos_type& pos)
   {
      if ( len > 99){return false;}
      
      char buf[100];
      memcpy(buf,cbuf,len );
      buf[len] ='\0';
      float ar[3] = {0.f,0.f,0.f};
      const char* delims[] = {",",",",";"};

      for ( size_t i = 0; i < 3; ++i){
         char* sptr = (i==0)? buf:nullptr;
         char* f = strtok(sptr,delims[i]);
         if ( f == nullptr ) {return false;}
         quan::detail::converter<float, char*> conv;
         ar[i] = conv(f);
         if(conv.get_errno()){
            return false;
         }
      }
      typedef quan::angle::deg deg;
      typedef quan::length::m m;
      pos = uav_pos_type{deg{ar[0]},deg{ar[1]},m{ar[2]}};
      return true;
   }

   bool parse_home_position(const char* buf,size_t len)
   {
      return parse_position(buf,len,telemetry::m_home_position);
   }

   // buf is a zero terminated string
   void parse_text_command(const char * buf)
   {
       size_t const len = strlen(buf);
       if ( len == 0) {
         return;
       }
       switch (buf[0]){

         case '$' :
               if (! parse_home_position(buf+1,len-1) ){
                  debug::serial_port::write("set home position failed\n");
               }
               else{
                 char buf1[100];
                 sprintf(buf1,"home pos set lat = %.6f, lon = %.6f, alt = %.1f\n",
                      static_cast<double>(quan::angle::deg{telemetry::m_home_position.lat}.numeric_value()),
                      static_cast<double>(quan::angle::deg{telemetry::m_home_position.lon}.numeric_value()),
                      static_cast<double>(telemetry::m_home_position.alt.numeric_value())
                  );
                 debug::serial_port::write(buf1);
               }
               break;

         case '~' : {
               uav_pos_type aircraft_position;
               if(!parse_position(buf+1,len-1,aircraft_position)){
                  debug::serial_port::write("set aircraft position failed\n");
               }else{
                 char buf1[100];
                 sprintf(buf1,"aircraft pos set lat = %.6f deg, lon = %.6f deg, alt = %.1f mm\n",
                     static_cast<double>(quan::angle::deg{aircraft_position.lat}.numeric_value()),
                      static_cast<double>(quan::angle::deg{aircraft_position.lon}.numeric_value()),
                      static_cast<double>(aircraft_position.alt.numeric_value())
                  );
                 telemetry::m_aircraft_position = aircraft_position;
                 telemetry::recalc();
                 
                 debug::serial_port::write(buf1);
               }
               break;
         }
         case 'Z' :{
               azimuth::encoder::zero();
               debug::serial_port::write("zeroed\n");
         }
         break;
         case 'E' :{
               azimuth::motor::enable();
               debug::serial_port::write("Azimuth Enabled\n");
         }
         break;
         case 'D' :{
               azimuth::motor::disable();
               debug::serial_port::write("Azimuth Disabled\n");
         }
         break;
         case 'P' :
            if ( len > 1){
               uint32_t const pos = atoi(buf+1);
               azimuth::motor::set_target_position(pos);
               char buf1[50];
               sprintf(buf1,"T <~ %ld : OK!\n",azimuth::motor::get_target_position());
               debug::serial_port::write(buf1);
            }
            else{
               debug::serial_port::write("expected uint");
            } 
         break;
         case 'H' :
            if ( len > 1){
               uint32_t pos = quan::constrain(atoi(buf+1),1000,2000);

               main_loop::set_elevation_servo(quan::time_<uint32_t>::us{pos});
               char buf1[50];
               sprintf(buf1,"H <~ %ld : OK!\n",main_loop::get_elevation_servo().numeric_value());
               debug::serial_port::write(buf1);
            }
            else{
               debug::serial_port::write("expected uint");
            } 
         break;
         case 'k': 
            if ( len > 3){
               switch (buf[1]){
                  case 'P' : 
                  case 'D' : {
                    quan::detail::converter<float,char*> conv;
                    float const v = conv(buf + 2);
                    if (conv.get_errno() ==0){
                        switch (buf[1]){
                           case 'P' : {
                              azimuth::motor::set_kP(v);
                              char buf1[50];
                              sprintf(buf1,"kP <~ %f : OK!\n",static_cast<double>(v));
                              debug::serial_port::write(buf1);
                           }
                           break;
                           case 'D':
                              azimuth::motor::set_kD(v);
                              char buf1[50];
                              sprintf(buf1,"kD  <~ %f : OK!\n",static_cast<double>(v));
                              debug::serial_port::write(buf1);
                           break;
                           default:
                              debug::serial_port::write("internal error\n");
                           break;
                        }
                    }else{
                        debug::serial_port::write("float conv error\n");
                    }
                  }
                  break;
                  default :
                     debug::serial_port::write("unknown command\n");
                  break;
               }
            }
            else{
               debug::serial_port::write("expctd kP or kD + float\n");
            } 
         break;
         case 'G' :
             if ( len > 1){
               switch (buf[1]){
                     case 'T':{
                        char buf1[50];
                        sprintf(buf1,"Pt = %ld\n",azimuth::motor::get_target_position());
                        debug::serial_port::write(buf1);
                     }
                     break;
                     case 'A':{
                        char buf1[50];
                        sprintf(buf1,"Pa = %ld\n",azimuth::motor::get_actual_position());
                        debug::serial_port::write(buf1);
                     }
                     break;
                     case 'p':{
                        char buf1[50];
                        sprintf(buf1,"kP = %f\n",static_cast<double>(azimuth::motor::get_kP()));
                        debug::serial_port::write(buf1);
                     }
                     break;
                     case 'd': {
                        char buf1[50];
                        sprintf(buf1,"kD = %f\n",static_cast<double>(azimuth::motor::get_kD()));
                        debug::serial_port::write(buf1);
                     }
                     break;
                     case 'M':{
                         quan::three_d::vect<float> const & mag_vect = raw_compass::get_vect();
                         char buf1[100];
                         sprintf(buf1,"mag vector = [%.3f,%.3f,%.3f]\n",
                           static_cast<double>(mag_vect.x),
                              static_cast<double>(mag_vect.y),
                                 static_cast<double>(mag_vect.z)
                         );
                         debug::serial_port::write(buf1);
                     }
                     break;
                     case 'm':{
                         quan::three_d::vect<float> const & mag_vect = raw_compass::get_raw();
                         char buf1[100];
                         sprintf(buf1,"raw mag vector = [%.3f,%.3f,%.3f]\n",
                           static_cast<double>(mag_vect.x),
                              static_cast<double>(mag_vect.y),
                                 static_cast<double>(mag_vect.z)
                         );
                         debug::serial_port::write(buf1);
                     }
                     break;
                     case 'f':{
                         quan::three_d::vect<float> const & mag_vect = raw_compass::get_offset();
                         char buf1[100];
                         sprintf(buf1,"mag offset = [%.3f,%.3f,%.3f]\n",
                           static_cast<double>(mag_vect.x),
                              static_cast<double>(mag_vect.y),
                                 static_cast<double>(mag_vect.z)
                         );
                         debug::serial_port::write(buf1);
                     }
                     break;
                     case 'B':{
                           quan::angle::deg bearing;
                           if (raw_compass::get_bearing(bearing)){
                              char buf1[200];
                              int32_t enc = azimuth::motor::bearing_to_encoder(bearing);
                              quan::angle::deg back = azimuth::motor::encoder_to_bearing(enc);
                              sprintf(buf1,"mag bearing = %.3f deg, encoder value = %ld, back = %.3f\n",
                                          bearing.numeric_value(), enc, back.numeric_value());
                              debug::serial_port::write(buf1);
                           }else{
                              debug::serial_port::write("get_bearing failed\n");
                           }
                     }
                     break;
                     default:
                        debug::serial_port::write("unknown get param\n");
                     break;
               }
             }
             else{
               debug::serial_port::write("expctd get param\n");
             }
         break;
         case 'M':{
            if ( len > 1){
               switch(buf[1]){
                  case '0':
                     raw_compass::set_strap(0);
                     debug::serial_port::write("set mag norm\n");
                  break;
                  case '+':
                     raw_compass::set_strap(1);
                     debug::serial_port::write("set mag +strp\n");
                  break;
                  case '-':
                     raw_compass::set_strap(-1);
                     debug::serial_port::write("set mag -strp\n");
                  break;
                  case '!':
                  break;
                  case '?':
                  break;
                  case '*':
                  break;
                  case 'f':{
                     quan::detail::converter<float,char*> conv;
                     float const v = conv(buf + 2);
                     if (conv.get_errno() == 0){
                        if( (v <= 1.0f) && (v > 0.0f) ){
                           raw_compass::set_filter(v);
                           char buf1[60];
                           sprintf(buf1,"raw mag filt set to %f\n",static_cast<double>(v));
                           debug::serial_port::write(buf1);
                        }else{
                           debug::serial_port::write("mag filt float range\n");
                        }
                     }else{
                        debug::serial_port::write("mag filt float invalid\n");
                     }
                  }
                  break;
                  default:
                     debug::serial_port::write("expctd M0, M+, M- mfxxx\n");
                  break;
               }
            }else {
               debug::serial_port::write("expctd M0, M+, M- mfxxx\n");
            }
         }
         break;   
         default:
            debug::serial_port::write("cmd not found\n");
         break;

       }
   }
}

void parse_commandline()
{
   constexpr uint32_t bufsize = 255;
   static char buffer[bufsize];
   static uint32_t index =0;
   if(debug::serial_port::in_avail()){
      if(index == bufsize){
         while(debug::serial_port::in_avail()){
            char ch = debug::serial_port::get();
            if(ch =='\n'){
               break;
            }
         }
         index = 0;
         debug::serial_port::write("command too long\n");
      }else{
         char ch = debug::serial_port::get();
         debug::serial_port::put(ch);
          
         if(ch == '\r'){
            debug::serial_port::write("#\n");
            buffer[index] = '\0';
            index = 0;
            parse_text_command(buffer);
         }else{
            buffer[index++] = ch;
         }
      }
   }
}