#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// Should match with your epaper module, size
#include <it8951/ed078kc2.h>

EpdFastSpi io;
Ed078Kc2 display(io);

//#include <Fonts/ubuntu/Ubuntu_M18pt8b.h>
#include <Fonts/ubuntu/Ubuntu_M8pt8b.h>
#include <Fonts/ubuntu/Ubuntu_M12pt8b.h>
//#include <Fonts/ubuntu/Ubuntu_M16pt8b.h>
//#include <Fonts/ubuntu/Ubuntu_M20pt8b.h>

extern "C"
{
   void app_main();
}

void demo_chars(){
 for (int i = 32; i <= 126; i++)
   {
      display.write(i);
   } 
 for (int i = 126+33; i <= 255; i++)
   {
      //printf("%d ",i);
      display.write(i);
   }
}

void app_main(void)
{
   printf("CalEPD version: %s This class is still in development and does not work!\n", CALEPD_VERSION);
   printf("Please use demo-it8951-lovyan.cpp example\n\n");
   // Show available Dynamic Random Access Memory available after display.init()
   printf("Fonts-demo. Free heap: %d (After epaper instantiation)\n", xPortGetFreeHeapSize());

   // Bootstrap epaper class
   display.init(false);
   
   display.clearScreen(155);

   display.fillCircle(200, 200, 400, 25);

   display.fillCircle(200, 400, 300, 225);
   //display.fillRect(100,100,100,100, 0);
   display.update();
   
    
   /* display.setCursor(50,100);
   display.setTextColor(EPD_WHITE);
   display.setFont(&Ubuntu_M12pt8b);
   display.println("Hello!");  

   
    */
   return;
}
