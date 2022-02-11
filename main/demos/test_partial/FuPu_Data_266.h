/*
	FuPu_Data.h
  User set of Image Data
*/

// 2.66"

#include "bosch.c"
#include "twinings.c"
#include "PS4.c"
#include "redbull.c"
#include "image_266_296x152_BW.c"

#include "0.c"
#include "1.c"
#include "2.c"
#include "3.c"
#include "4.c"
#include "5.c"
#include "6.c"
#include "7.c"
#include "8.c"
#include "9.c"
#include "w.c"
const unsigned char* fastImageSet[] = {
	(uint8_t *) & image_266_296x152_BW_mono, (uint8_t *) & Img_bosch,
	(uint8_t *) & Img_bosch, (uint8_t *) & Img_twinings,
	(uint8_t *) & Img_twinings, (uint8_t *) & Img_PS4,
	(uint8_t *) & Img_PS4, (uint8_t *) & Img_redbull,
	(uint8_t *) & Img_redbull, (uint8_t *) & image_266_296x152_BW_mono,
	(uint8_t *) & image_266_296x152_BW_mono, (uint8_t *) & Img_bosch,
	(uint8_t *) & Img_bosch, (uint8_t *) & Img_twinings,
	(uint8_t *) & Img_twinings, (uint8_t *) & Img_PS4,
	(uint8_t *) & Img_PS4, (uint8_t *) & Img_redbull,
	(uint8_t *) & Img_redbull, (uint8_t *) & image_266_296x152_BW_0x00};
	
const unsigned char* partialImageSet[] = {
	(uint8_t *) & Img_w, (uint8_t *) & Img_0,
	(uint8_t *) & Img_0, (uint8_t *) & Img_1,
	(uint8_t *) & Img_1, (uint8_t *) & Img_2,
	(uint8_t *) & Img_2, (uint8_t *) & Img_3,
	(uint8_t *) & Img_3, (uint8_t *) & Img_4,
	(uint8_t *) & Img_4, (uint8_t *) & Img_5,
	(uint8_t *) & Img_5, (uint8_t *) & Img_6,
	(uint8_t *) & Img_6, (uint8_t *) & Img_7,
	(uint8_t *) & Img_7, (uint8_t *) & Img_8,
	(uint8_t *) & Img_8, (uint8_t *) & Img_9,
	(uint8_t *) & Img_9, (uint8_t *) & Img_w};

uint8_t fastImageSet_Size = sizeof(fastImageSet)/sizeof(*fastImageSet);
long partialWindowSize = 280;
uint8_t partialImageSet_config[5] = {sizeof(partialImageSet)/sizeof(*partialImageSet), 0x07, 0x0b, 0x32 ,0x6a};
