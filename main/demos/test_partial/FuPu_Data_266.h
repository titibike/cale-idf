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
	
uint8_t fastImageSet_Size = sizeof(fastImageSet)/sizeof(*fastImageSet);
