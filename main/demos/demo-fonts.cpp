#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// Should match with your epaper module, size
//#include <gdew042t2.h>
//#include <gdew0583t7.h>
//#include <gdew075T7.h>
//#include <gdew075T8.h>
//#include <gdew027w3.h>
//#include <gdeh0213b73.h>

/* Ecovelo */
#include "eco_se2266_color.h"
// Single SPI EPD
//EpdSpi io;
//Gdew075T8 display(io);
//Gdew075T7 display(io);
//Gdew042t2 display(io);
//Gdew0583T7 display(io);
//Gdew027w3 display(io);
//Gdeh0213b73 display(io);

// Multi-SPI 4 channels EPD only - 12.48 Epaper display
// Please note that in order to use this big buffer (160 Kb) on this display external memory should be used
// Otherwise you will run out of DRAM very shortly!
//#include "wave12i48.h" // Only to use with Edp4Spi IO
//#include "wave12i48BR.h" // Only to use with Edp4Spi IO, Black Red model

EpdSpi io;

//Wave12I48RB display(io);
EcoSE2266 display(io);


// FONT used for title / message body - Only after display library
// Please check https://learn.adafruit.com/adafruit-gfx-graphics-library/using-fonts#adding-new-fonts-2002831-18
// Converting fonts with ümlauts: ./fontconvert *.ttf 18 32 252

//#include <Fonts/ubuntu/Ubuntu_M18pt8b.h>
#include <Fonts/ubuntu/Ubuntu_M8pt8b.h>
#include <Fonts/ubuntu/Ubuntu_M12pt8b.h>
#include <Fonts/ubuntu/Ubuntu_M48pt8b.h>
#include <Fonts/FreeMono24pt7b.h>
//#include <Fonts/ubuntu/Ubuntu_M16pt8b.h>
//#include <Fonts/ubuntu/Ubuntu_M20pt8b.h>

#include "test_partial/FuPu_Data_266.h"
#include "test_partial/FuPu_LUT_266.h"

#define BW_monoBuffer        (uint8_t *) & image_266_296x152_BW_mono
#define BW_0x00Buffer        (uint8_t *) & image_266_296x152_BW_0x00

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
   printf("begin init \n");
   display.init(false);
   printf("end init \n");
   vTaskDelay(100/portTICK_RATE_MS);
   char data_char[100];
   #if 0
   
      vTaskDelay(200/portTICK_RATE_MS);
      printf("test partial refresh\n");
      display.fillScreen(EPD_WHITE);
      //display.globalUpdate(BW_monoBuffer, BW_0x00Buffer);
      display.update();
      vTaskDelay(1000/portTICK_RATE_MS);
      printf("test global refresh\n");
      display.updateLUT(&ltb_custom);
      display.fastUpdateInit();

      display.setTextColor(EPD_BLACK);
      display.setFont(&Ubuntu_M12pt8b);
      display.setRotation(3);
      display.setCursor(30,20);
      for (int i=0;i<300;i=i+50){
         printf("test partial refresh: %d\n",i); 
         display.fastUpdateInit();
         //vTaskDelay(1000/portTICK_RATE_MS);
         
         //display.setCursor(i+30,20);
         sprintf(data_char,"Hello %d",i);
         printf("char: %s\n",data_char);
         display.println(data_char);
         display.fastUpdate();
      //display.fastUpdateTest(fastImageSet, fastImageSet_Size,1);
   }
   #endif

   #if 1 /* Partial*/
   
      vTaskDelay(200/portTICK_RATE_MS);
      printf("test partial refresh\n");
      display.fillScreen(EPD_WHITE);
      //display.globalUpdate(BW_monoBuffer, BW_0x00Buffer);
      display.update();
      vTaskDelay(1000/portTICK_RATE_MS);
      printf("test global refresh\n");
      display.updateLUT(&ltb_custom);
      display.fastUpdateInit();
      display.setTextColor(EPD_BLACK);
      display.setFont(&Ubuntu_M12pt8b);
      display.setCursor(56,50);
      //display.println("O");
      //display.printBuffer();
      printf("draw rect\n");
      int w= 20;
      int h=20; 
      int x=10;
      int y=10;
      //display.drawRect(x,y,w,h,EPD_BLACK);
      //display.printBuffer();
      //for (int c=0;c<4;c++){
      display.fillScreen(EPD_WHITE);
      display.update();
        display.setRotation(3);
         display.drawRect(x,y,w,h,EPD_BLACK);
         //display.update();
         //display.printBuffer();
         display.updateWindow(x,y,w,h,true);
      //}
      #if 0
      for(int i=0;i<50000;i++){
         sprintf(data_char,"%d",i);
         printf("char: %s\n",data_char);
         display.println(data_char);
         display.updateWindow(56,50,40,56,false);
         //display.fastUpdate();
         //display.partialUpdateTest(partialImageSet, partialImageSet_config, partialWindowSize, 1);
      }
      #endif
   
   #endif

   #if 0
   printf(" Test Sushi \n");
  display.testbuff(0);
   display.update();
    display.testbuff(1);
    display.update();
    vTaskDelay(2000/portTICK_RATE_MS);
    #endif



   #if 0
    printf("print Rectangle \n");
    display.setRotation(0);
    
    //display.setTextColor(EPD_BLACK);
    //display.setCursor(15,20);
    //display.println("B");
    display.globalUpdate(BW_monoBuffer, BW_0x00Buffer);

    //display.drawRect(10,10,22,20,EPD_BLACK);
    //display.update();
     //Turn off DC/DC
   vTaskDelay(2000/portTICK_RATE_MS);  
   display.fillScreen(EPD_WHITE);
   display.update();

    display.updateLUT(&ltb_custom); 
   display.fastUpdateInit();
   
   //vTaskDelay(2000/portTICK_RATE_MS);  

   for (int i=0;i<20;i++){
       display.drawPixel(i,10,EPD_WHITE);
    }
    display.fastUpdate();
   printf("previous buff ==buffer ? : %d\n ", memcmp(display._previous_buffer,display._buffer,EcoSE2266_BUFFER_SIZE));
   for (int i=20;i<40;i++){
       display.drawPixel(i,20,EPD_WHITE);
    }
  
    printf("previous buff ==buffer ? : %d\n ", memcmp(display._previous_buffer,display._buffer,EcoSE2266_BUFFER_SIZE));
   display.fastUpdate();
 

   for (int i=40;i<60;i++){
       display.drawPixel(i,30,EPD_WHITE);
    }

   display.fastUpdate();

   for (int i=60;i<100;i++){
       display.drawPixel(i,40,EPD_WHITE);
    }
   display.fastUpdate(); 
   vTaskDelay(200/portTICK_RATE_MS); 

  //display.updateWindow(0,0,EcoSE2266_WIDTH,EcoSE2266_HEIGHT);
   printf("\n end update windows \n");
   /*
   vTaskDelay(2000/portTICK_RATE_MS);  

   for (int i=0;i<10;i++){
       display.drawPixel(i,0,EPD_BLACK);
    }
   display.update();
   vTaskDelay(2000/portTICK_RATE_MS);  */
   #endif
  
}
