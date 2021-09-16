#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// Should match with your epaper module, size
#include <it8951/epdfastspi.h>
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
   printf("CalEPD version: %s\n", CALEPD_VERSION);
   // Show available Dynamic Random Access Memory available after display.init()
   printf("Fonts-demo. Free heap: %d (After epaper instantiation)\nDRAM     : %d\n", 
   xPortGetFreeHeapSize(),heap_caps_get_free_size(MALLOC_CAP_8BIT));

   // Bootstrap epaper class
   display.init(false);
   // Store your epapers all white, just turn true:
   if (false) {
     display.fillScreen(EPD_WHITE); 
     display.update();
     return;
   }

   display.update();
   vTaskDelay(700); // short delay to demonstrate red color working
   display.fillScreen(EPD_WHITE);

   display.fillCircle(300,300, 100, EPD_BLACK);

   display.setCursor(10,40);
   display.setTextColor(EPD_BLACK);
   
   display.setFont(&Ubuntu_M12pt8b);

   display.println("German characters test");
   display.println("° äöü ÄÖÜ ß");
   display.println("Löwen, Bären, Vögel und Käfer sind Tiere. Völlerei lässt grüßen! Heute ist 38° zu warm.");
   display.println("");
   display.println("Spanish / French characters test");
   display.println("æçèé êëìí ï ñ");
   display.println("La cigüeña estaba sola en la caña. Estás allí?");
   display.newline(); // new way to add a newline
   
   // German sentence
   display.setFont(&Ubuntu_M8pt8b);
   display.print("Ubuntu 8pt");
   demo_chars();


   display.println("");
   display.print("\nUbuntu 12pt");
   display.setFont(&Ubuntu_M12pt8b);
   display.setTextColor(EPD_WHITE); // test white on the black circle
   demo_chars();

   // Let's draw one 100px radius circle Black and another on the right 120px radius Red
   display.fillCircle(300,650, 165, EPD_BLACK); // test bottom left quadrant

   display.fillCircle(600,300, 120, EPD_BLACK);

   display.fillCircle(900,700, 150, EPD_BLACK);  // test bottom right quadrant

   display.update();
}
