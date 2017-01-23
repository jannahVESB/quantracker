
#include "system/resources.hpp"
#include <quan/stm32/flash.hpp>
#include <quan/error.hpp>
#include <quan/user.hpp>

void parse_commandline();

namespace {

     
      typedef gcs_serial sp_task;

      void do_cmdline()
      {    
         sp_task::write("Quantracker Interactive mode\n");

         for (;;){
            sp_task::write("P for commandline\n");
            sp_task::write("F for flash menu\n");
            switch (sp_task::get()){

               case 'P':
               case 'p':{
                  for (;;){
                     parse_commandline();
                  }
                  break;
               }
               case 'C':
               case 'c':{
    
                  break;
               }
               case 'F':
               case 'f':{
                  sp_task::write("Quantracker Ground OSD 2.1\n");
                  sp_task::write("~~~~Flash menu mode~~~~\n");
      
                  quan::stm32::flash::flash_menu();
                  quan::report_errors();
                  break;
               }
               default:
               break;
            }
         }
      }


      void tracker_mode(void * param)
      {
         sp_task::enable();
         bool cmdline_mode = true;
         sp_task::write("Quantracker Ground V1.1\n");

         // wait for a while?
         // look for 3 x ret
         for (int count = 0;count < 3; ++count){
           if ( sp_task::get() != '\r'){
               cmdline_mode = false;
               break;
           }
         }
         if ( cmdline_mode){
            do_cmdline();
         }else{
            // start tracking
            sp_task::write("start tracking\n");
    
         }
         // delete the task
         for (;;){
            asm volatile ("nop":::);
         }
      }

      char dummy_param  =0;
      TaskHandle_t task_handle = NULL;
}

void create_tracker_task()
{

   sp_task::setup<9600>(local_interrupt_priority::sliprings_serial_port);
   xTaskCreate(
      tracker_mode,"tracker_mode", 
      5000,
      &dummy_param,
      local_task_priority::tracker_mode,
      &task_handle
   );
}