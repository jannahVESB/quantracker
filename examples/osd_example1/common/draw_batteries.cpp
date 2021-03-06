
#include <cstdio>
#include <quan/uav/osd/features_api.hpp>
#include "osd.hpp"
#include <cmath>
#include <quan/min.hpp>

using namespace quan::uav::osd;

void draw_batteries()
{
   
   //want to draw voltage inside battery
   // voltage up to 25 V with 1 decimal
   // so 12.2V == 5 chars
   // get font, get the size of
   // size of box = ( 5 * x font.y + border * 2)

   
   pxp_type const pos = 
      {-155,
      (( get_video_mode() == video_mode::pal)
      ?-130:-80)};

   font_ptr font = get_font(FontID::TINY_NUM);
   if ( font){
      size_type font_size = get_size(font);
      size_type const border = {2,1};
      pxp_type pt_array[]= {
            {-border.x,-border.y}
            ,{font_size.x * 5 + border.x +1,-border.y}
            ,{font_size.x * 5 + border.x +1,- border.y + (font_size.y + 2 * border.y)/3}
            ,{font_size.x * 5 + border.x +5,- border.y +(font_size.y + 2 * border.y)/3}
            ,{font_size.x * 5 + border.x +5,(font_size.y + 2 * border.y +1) * 2/3}
            ,{font_size.x * 5 + border.x +1,(font_size.y + 2 * border.y +1)*2/3}
            ,{font_size.x * 5 + border.x +1, border.y + (font_size.y + 2 * border.y)} 
            ,{- border.x, border.y + (font_size.y + 2 * border.y)}
       };
     
       uint32_t num_points = (sizeof(pt_array) / sizeof(pxp_type));
       enum dirs{ inside_left, inside_right, inside_up,inside_down};
       dirs const dirs_array[]  ={
            inside_up,inside_left,inside_up,inside_left,inside_down,inside_left,inside_down, inside_right
       };
       size_type dirs_move[] = {{-1,0},{1,0},{0,1},{0,-1}};
       for ( uint32_t i = 1; i < num_points; ++i){
         for( int32_t j = -1;j < 2; ++j){
            size_type const move = dirs_move[dirs_array[i-1]] * j;
            colour_type colour = (j == 0)?colour_type::white: colour_type::black;
            pxp_type p1 = pos + move + pt_array[i-1];
            pxp_type p2 = pos + move + pt_array[i];
            if ( p1.y == p2.y){
               pxp_type sp{quan::min(p1.x,p2.x),p1.y};
               uint32_t len = abs(p1.x- p2.x);
               draw_horizontal_line(sp,len,colour);
            }else{
               draw_line(p1,p2,colour );
            }
         }
       }
       for( int32_t j = -1;j < 2; ++j){
         size_type const move = dirs_move[dirs_array[num_points-1]] * j;
         colour_type colour = (j == 0)?colour_type::white: colour_type::black;
         draw_line(pos + move + pt_array[num_points -1], pos + move + pt_array[0],colour );
       }

       char buf [30];
       quan::voltage_<double>::V cur_battery_voltage = get_battery_voltage(0);
       sprintf(buf,"%4.1fv",cur_battery_voltage.numeric_value());
       draw_text(buf,pos + pxp_type{1,3},font);
   }
   
   
}