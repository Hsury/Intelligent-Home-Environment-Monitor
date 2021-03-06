/*
 * 项目名称：智能家用环境监测仪
 * 版本：2016/11/06
 */
#include <I2Cdev.h>
#include <MPU6050.h>
#include <EEPROM.h>
#include <Wire.h>
#include "math.h"
#include "DHT.h"
#include "U8glib.h"

#define Buzzer 13
#define LEDGreen 10
#define LEDBlue 11
#define IRSensor 9
#define LDSensor 0
#define RDSensor 1

boolean ledactive = 1;
boolean ishand = 1;
boolean nosleep = 1;
boolean toalarm = 1;
boolean rotated = 0;
boolean changemode = 0;
boolean BuzzerSwitch = 1;
float TData, HData, RangeDown, RangeUp, basictemp;
int sleepInterval = 30 * 1000;
int menu = 5;
int ledlevel = 0;
int light = 0;
int LD, RD;
int stat[3];
int latestmove = 0;
int16_t ax, ay, az;
int16_t gx, gy, gz;
float Ax, Ay, Az;
float Angel_accX, Angel_accY, Angel_accZ, A, B, C;
long timeStamp[8];

U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_NONE);
DHT dht;
MPU6050 accelgyro;

static unsigned char u8g_welcome1[] U8G_PROGMEM = //欢迎1
{
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x04, 0x01, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xC0, 0x0F, 0x06, 0x01, 0x00, 0x04, 0xC0, 0xFF, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC0, 0x5F, 0x08, 0x12, 0x31, 0xFC, 0xFF, 0x43, 0x10, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x42, 0x08, 0x21, 0x0D, 0x04, 0x00, 0x42, 0x10, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x42, 0x88, 0x7F, 0x43, 0x04, 0x00, 0x42, 0x10, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xE0, 0x5F, 0x08, 0xC0, 0x41, 0xF0, 0xFF, 0x40, 0x10, 0x04, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x43, 0x08, 0x00, 0x41, 0x00, 0x03, 0xC0, 0xFF, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xCD, 0x0F, 0x3F, 0x3E, 0x80, 0x81, 0x40, 0x10, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC0, 0x18, 0x80, 0x20, 0x00, 0x60, 0x62, 0x40, 0x10, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xA0, 0xFF, 0x07, 0x3F, 0x01, 0x1C, 0x1B, 0x20, 0x10, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x84, 0x20, 0x61, 0x80, 0x24, 0xE0, 0xFF, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x84, 0x20, 0x19, 0x60, 0x26, 0x20, 0x10, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xFF, 0x07, 0x3F, 0x07, 0x1C, 0x47, 0x20, 0x10, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x84, 0x20, 0x41, 0x80, 0xC5, 0x20, 0x10, 0x04, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x80, 0x00, 0x84, 0x20, 0x41, 0x60, 0x04, 0x31, 0x10, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xFF, 0x87, 0x20, 0x41, 0x18, 0x04, 0x12, 0x10, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x04, 0x38, 0x3E, 0x84, 0x03, 0x10, 0x90, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x40, 0x08, 0x00, 0x00, 0x04, 0x41, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF8, 0x0F, 0x01, 0x01, 0x44, 0x08, 0x98, 0x9F, 0x04, 0x41, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF0, 0x41, 0x00, 0xF9, 0x3F, 0x44, 0x04, 0xB0, 0x90, 0x04, 0x89, 0x08, 0x00, 0x00,
  0x00, 0x00, 0x40, 0x40, 0x00, 0x21, 0x08, 0x44, 0xFC, 0xA1, 0x90, 0x84, 0x88, 0x08, 0x00, 0x00, 0x00, 0x00, 0x40, 0x40, 0xC0, 0x43, 0x04, 0x44, 0x02, 0x80, 0x94, 0x84, 0x08, 0x08, 0x00, 0x00, 0x00, 0x00, 0x40, 0x20, 0x00, 0xF9, 0x3F, 0x44, 0x02, 0x88, 0x94, 0xC4, 0x08, 0x08, 0x00, 0x00, 0x00, 0x00, 0x40, 0x60, 0x00, 0x01, 0x00, 0x44, 0x31, 0x98, 0x94, 0xC4, 0x10, 0x04, 0x00, 0x00, 0x00, 0x00, 0xF0, 0x51, 0x01, 0xF1, 0x1F, 0xC4, 0x60, 0x90, 0x94, 0xA4, 0x10, 0x04, 0x00, 0x00, 0x00, 0x00, 0x40, 0x50, 0x02, 0x11, 0x10, 0x40, 0x00, 0x80, 0x94, 0x94, 0x30, 0x02, 0x00, 0x00, 0x00, 0x00, 0x40, 0x48, 0x02, 0xF1, 0x1F, 0x00, 0x00, 0x80, 0x94, 0x84, 0x20, 0x02, 0x00, 0x00, 0x00, 0x00, 0x40, 0x48, 0x04, 0x11, 0x10, 0xF8, 0x7F, 0xA0, 0x92, 0x84, 0x60, 0x01, 0x00, 0x00,
  0x00, 0x00, 0x40, 0x44, 0x08, 0x15, 0x10, 0x88, 0x44, 0xB0, 0x92, 0x84, 0xC0, 0x01, 0x00, 0x00, 0x00, 0x00, 0x40, 0x40, 0xC8, 0xF3, 0x1F, 0x88, 0x44, 0x10, 0x82, 0x84, 0xC0, 0x01, 0x00, 0x00, 0x00, 0x00, 0xC0, 0x41, 0x00, 0x40, 0x02, 0x88, 0x44, 0x10, 0x0E, 0x84, 0x60, 0x03, 0x00, 0x00, 0x00, 0x00, 0x30, 0x40, 0x00, 0x40, 0x42, 0x88, 0x44, 0x10, 0x19, 0x84, 0x30, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x00, 0x30, 0x22, 0x88, 0x44, 0x88, 0x20, 0x84, 0x0C, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x00, 0x0E, 0x3E, 0xFE, 0xFF, 0x49, 0x80, 0x83, 0x06, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

static unsigned char u8g_welcome2[] U8G_PROGMEM = //欢迎2
{
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x07, 0xC0, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC0, 0x7F, 0x70, 0x0C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0xE0, 0x1C, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x80, 0x07, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x20, 0x80, 0x07, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x01, 0x00, 0x08, 0x60, 0xC0, 0x1C, 0x10, 0x00, 0x00, 0x00, 0x07, 0xC0, 0x08, 0x00, 0x08, 0x00, 0x01, 0x00, 0x09, 0x40, 0x60, 0x38, 0x10, 0x80, 0x01, 0xC0, 0x04, 0x20, 0x08, 0x40, 0x08, 0x00, 0x01, 0x20, 0x05, 0x40, 0x20, 0x60, 0x10, 0x80, 0x01, 0x20, 0x02, 0x00, 0x0A, 0x40, 0x08, 0x00, 0x01, 0x60, 0x0E, 0xC0, 0x1E, 0x40, 0x08, 0x80, 0x0F, 0x00, 0x03, 0xC0, 0x0C, 0x40, 0x1C, 0x00, 0x09, 0xC0, 0x33, 0x80, 0x3F, 0x80, 0x08, 0xF0, 0x19, 0x00, 0x3E, 0x78, 0x0A, 0xC0, 0x0F, 0x00, 0x0F, 0x20, 0x62, 0xA2, 0x21, 0xDE, 0x67, 0xD0, 0x1B, 0xC0, 0x03, 0xE0, 0x0E, 0x70, 0x04, 0xE0, 0x07, 0xC8, 0x1D, 0xA2, 0x6D, 0x02, 0x95, 0xD0, 0x09, 0x78, 0x02, 0x50, 0x7C, 0x50, 0x1E, 0xE0, 0x01, 0x48, 0x02,
  0xA2, 0x71, 0x0E, 0x13, 0xE0, 0x0B, 0x08, 0x02, 0xC8, 0x0B, 0x40, 0x09, 0x80, 0x03, 0x00, 0x3E, 0xA2, 0x6D, 0x10, 0x93, 0xE0, 0x0F, 0x00, 0x02, 0x4C, 0x08, 0x70, 0x0E, 0x80, 0x06, 0xE0, 0x03, 0x1C, 0x33, 0x9E, 0x65, 0x80, 0x81, 0x00, 0x02, 0x40, 0x08, 0x58, 0x0E, 0xC0, 0x0C, 0x10, 0x02, 0x80, 0x3E, 0xC0, 0x08, 0x00, 0xC1, 0x00, 0x02, 0x40, 0x10, 0xC8, 0x33, 0x60, 0x30, 0x00, 0x02, 0xC0, 0x30, 0x60, 0x18, 0x00, 0xFE, 0x00, 0x02, 0x00, 0x10, 0x60, 0xF0, 0x38, 0xF0, 0x01, 0x02, 0x40, 0x70, 0x30, 0x10, 0x00, 0x00, 0x80, 0x03, 0x00, 0x10, 0x40, 0x00, 0x00, 0x00, 0x80, 0x03, 0x60, 0xC0, 0x38, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x80, 0x0F, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x20, 0x00, 0x07, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x80, 0x1F, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0xE0, 0x3C, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x7F, 0xF0, 0x1F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0x80, 0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

static unsigned char u8g_menu0[] U8G_PROGMEM = //温度摄氏
{
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xE4, 0x0F, 0x18, 0x00, 0xA0, 0x3F, 0xC0, 0x03, 0x28, 0x88, 0xFF, 0x01, 0x22, 0x11, 0x3F, 0x08, 0xE0, 0x8F, 0x44, 0x00, 0x71, 0x1F, 0x21, 0x10, 0x24, 0x88, 0xFF, 0x01, 0x21, 0x11, 0x21, 0x10, 0xE8, 0x8F, 0x44, 0x80, 0xE0, 0x3F, 0x21, 0x20, 0x20, 0x88, 0x7C, 0x80, 0x20, 0x10, 0xFF, 0x27,
  0x08, 0x80, 0x00, 0x80, 0xB0, 0x3F, 0x21, 0x20, 0xE8, 0x8F, 0xFE, 0x80, 0xA0, 0x26, 0x21, 0x20, 0xA8, 0x8A, 0x44, 0x80, 0x20, 0x19, 0x51, 0x24, 0xA4, 0x4A, 0x38, 0x80, 0xA1, 0x1A, 0xCF, 0x34, 0xF4, 0xDF, 0xC7, 0x03, 0x71, 0x64, 0x81, 0x13, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

static unsigned char u8g_menu1[] U8G_PROGMEM = //温度华氏
{
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xE4, 0x0F, 0x18, 0x00, 0x40, 0x02, 0xC0, 0x03, 0x28, 0x88, 0xFF, 0x01, 0x22, 0x22, 0x3F, 0x08, 0xE0, 0x8F, 0x44, 0x00, 0x21, 0x1A, 0x21, 0x10, 0x24, 0x88, 0xFF, 0x01, 0x31, 0x06, 0x21, 0x10, 0xE8, 0x8F, 0x44, 0x80, 0x28, 0x43, 0x21, 0x20, 0x20, 0x88, 0x7C, 0x80, 0xE0, 0x42, 0xFF, 0x27,
  0x08, 0x80, 0x00, 0x80, 0x20, 0x3E, 0x21, 0x20, 0xE8, 0x8F, 0xFE, 0x80, 0x00, 0x01, 0x21, 0x20, 0xA8, 0x8A, 0x44, 0x80, 0xF8, 0x7F, 0x51, 0x24, 0xA4, 0x4A, 0x38, 0x80, 0x01, 0x01, 0xCF, 0x34, 0xF4, 0xDF, 0xC7, 0x03, 0x01, 0x01, 0x81, 0x13, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

static unsigned char u8g_menu2[] U8G_PROGMEM = //空气湿度
{
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x40, 0x00, 0xF3, 0x07, 0x0C, 0x00, 0x00, 0xFF, 0xE3, 0x3F, 0x14, 0xC4, 0xFF, 0x00, 0x00, 0x01, 0x22, 0x00, 0xF0, 0x47, 0x22, 0x00, 0x00, 0x48, 0xD0, 0x3F, 0x11, 0xC4, 0xFF, 0x00, 0x00, 0x86, 0x19, 0x00, 0xF2, 0x47, 0x22, 0x00, 0x80, 0x01, 0xFA, 0x1F, 0x00, 0x40, 0x3E, 0x00,
  0x00, 0xFE, 0x00, 0x10, 0x48, 0x49, 0x00, 0x00, 0x00, 0x10, 0x00, 0x10, 0x5A, 0x49, 0x7F, 0x00, 0x00, 0x10, 0x00, 0x50, 0x52, 0x45, 0x22, 0x00, 0x00, 0x10, 0x00, 0x50, 0x41, 0x21, 0x1C, 0x00, 0x00, 0xFF, 0x03, 0x60, 0xF9, 0xEF, 0xE3, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

static unsigned char u8g_menu3[] U8G_PROGMEM = //水平仪
{
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0xFE, 0x1F, 0x12, 0x00, 0x00, 0x00, 0x00, 0x08, 0x80, 0x00, 0x27, 0x01, 0x00, 0x00, 0x00, 0x88, 0x8C, 0x08, 0x45, 0x01, 0x00, 0x00, 0xC0, 0x5B, 0x90, 0x06, 0x05, 0x01, 0x00, 0x00, 0x00, 0x3A, 0x80, 0x80, 0x89, 0x00, 0x00, 0x00, 0x00, 0x2A, 0xFE, 0xDF, 0x89, 0x00, 0x00,
  0x00, 0x00, 0x29, 0x80, 0x40, 0x51, 0x00, 0x00, 0x00, 0x80, 0x49, 0x80, 0x00, 0x21, 0x00, 0x00, 0x00, 0x80, 0x88, 0x80, 0x00, 0x71, 0x00, 0x00, 0x00, 0x40, 0x08, 0x81, 0x00, 0x89, 0x00, 0x00, 0x00, 0x00, 0x0E, 0x80, 0x00, 0x07, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

static unsigned char u8g_menu4[] U8G_PROGMEM = //温控策略
{
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF9, 0x43, 0x08, 0x08, 0x01, 0x10, 0x00, 0x00, 0x0A, 0x42, 0x10, 0xFC, 0xEF, 0xFB, 0x00, 0x00, 0xF8, 0xF3, 0xFE, 0x4C, 0xA1, 0x8A, 0x00, 0x00, 0x09, 0x42, 0x82, 0x52, 0xA2, 0x76, 0x00, 0x00, 0xFA, 0x43, 0x6C, 0xFE, 0xEF, 0x73, 0x00, 0x00, 0x08, 0xC2, 0x87, 0x40, 0xA0, 0x8E, 0x01,
  0x00, 0x02, 0x70, 0xFE, 0xFC, 0xA7, 0xFA, 0x00, 0x00, 0xFA, 0x43, 0x10, 0x44, 0xA4, 0x8A, 0x00, 0x00, 0xAA, 0x42, 0x10, 0xE4, 0xE6, 0x8B, 0x00, 0x00, 0xA9, 0x42, 0x10, 0x58, 0x23, 0xFA, 0x00, 0x00, 0xFD, 0x77, 0xFF, 0x46, 0x0C, 0x88, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

static unsigned char u8g_menu5[] U8G_PROGMEM = //连接情况
{
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x21, 0x20, 0x0C, 0x84, 0x20, 0xFE, 0x00, 0x00, 0x22, 0x20, 0x7F, 0xF4, 0x4F, 0x82, 0x00, 0x00, 0xFE, 0xFB, 0x22, 0x8C, 0x80, 0x82, 0x00, 0x00, 0x10, 0x20, 0x12, 0xFD, 0x07, 0x82, 0x00, 0x80, 0x4B, 0xA0, 0x7F, 0x85, 0x00, 0xFE, 0x00, 0x00, 0xFA, 0x63, 0x04, 0xFD, 0x8F, 0xAA, 0x00,
  0x00, 0x42, 0xB8, 0x7F, 0x14, 0x84, 0x28, 0x00, 0x00, 0xFA, 0x23, 0x21, 0xF4, 0x47, 0x28, 0x00, 0x00, 0x42, 0x20, 0x16, 0x14, 0x44, 0x24, 0x01, 0x00, 0x47, 0x20, 0x3C, 0xF4, 0x27, 0x26, 0x01, 0x80, 0xFC, 0xBB, 0x43, 0x14, 0x04, 0xE1, 0x01, 0x00, 0x00, 0x00, 0x00, 0x94, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

void setup()
{
  int i, x;
  pinMode(Buzzer, OUTPUT);
  pinMode(IRSensor, INPUT);
  pinMode(LEDBlue, OUTPUT);
  pinMode(LEDGreen, OUTPUT);
  dht.setup(8);
  Wire.begin();
  Serial.begin(9600);
  accelgyro.initialize();
  Serial.println("UESTC Smart Homekit V1.1");
  u8g.setFont(u8g_font_courR14);
  u8g.firstPage(); //开机画面1
  do {
    u8g.drawXBMP( 0, 0, 128, 64, u8g_welcome1);
  } while (u8g.nextPage());
  delay(1500);
  u8g.firstPage(); //开机画面2
  do {
    u8g.drawXBMP( 0, 16, 128, 32, u8g_welcome2);
  } while (u8g.nextPage());
  delay(1500);
  nosleep = EEPROM.read(0);
  toalarm = EEPROM.read(1);
  ledactive = EEPROM.read(2);
  EEPROM.get(10, RangeUp);
  EEPROM.get(20, RangeDown);
  EEPROM.get(30, basictemp);
  for (i = 0; i <= 8; i++) timeStamp[i] = -60000;
  digitalWrite(Buzzer, HIGH); //开启蜂鸣器
  delay(50);
  digitalWrite(Buzzer, LOW); //关闭蜂鸣器
  //cleaneeprom();
  //ishand = 0;
}

void loop()
{
  getData();
  handScanner();
  COMController();
  ScrController();
  isAlarm();
  LEDController();
}

void getData () {
  accelgyro.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
  Ax = ax / 16384.00;
  Ay = ay / 16384.00;
  Az = az / 16384.00;
  Angel_accX = atan(Ax / sqrt(Az * Az + Ay * Ay)) * 180 / 3.14;
  Angel_accY = atan(Ay / sqrt(Ax * Ax + Az * Az)) * 180 / 3.14;
  Angel_accZ = atan(Az / sqrt(Ax * Ax + Ay * Ay)) * 180 / 3.14;
  B = -Angel_accX + 32 + 1;
  A = -Angel_accY + 64 + 1;
  LD = (6762 / (analogRead(LDSensor) - 9)) - 4;
  RD = (6762 / (analogRead(RDSensor) - 9)) - 4;
  if ((LD > 50) || (RD > 50)) {
    LD = 0;
    RD = 0;
  }
  if (millis() - timeStamp[1] > 2000) {
    TData = dht.getTemperature();
    HData = dht.getHumidity();
    timeStamp[1] = millis();
  }
  if (digitalRead(IRSensor) == HIGH || timeStamp[0] < 0) timeStamp[0] = millis();
  if ((millis() - timeStamp[0] > sleepInterval) && nosleep == 0) stat[0] = 1; //节电
  else {
    stat[0] = 0; //正常
    if (TData + basictemp < RangeDown) stat[1] = 1; //低温
    else if (TData + basictemp > RangeUp) stat[1] = 2; //高温
    else stat[1] = 0;
  }
}

void ScrController() {
  switch (stat[0]) {
    case 0: //正常显示
      switch (stat[2]) {
        case 0: //温度页面1
          if (millis() - timeStamp[2] > 1000) {
            u8g.firstPage();
            do {
              drawFrame();
              u8g.drawXBMP( 32, 0, 64, 16, u8g_menu0);
              u8g.setPrintPos(25, 42);
              u8g.print(TData + basictemp, 1);
              u8g.print(" ");
              u8g.print((char)176);
              u8g.print("C");
            } while (u8g.nextPage());
            timeStamp[2] = millis();
          }
          break;

        case 1: //温度页面2
          if (millis() - timeStamp[2] > 1000) {
            u8g.firstPage();
            do {
              drawFrame();
              u8g.drawXBMP( 32, 0, 64, 16, u8g_menu1);
              u8g.setPrintPos(25, 42);
              u8g.print((TData + basictemp) * 9 / 5 + 32, 1);
              u8g.print(" ");
              u8g.print((char)176);
              u8g.print("F");
            } while (u8g.nextPage());
            timeStamp[2] = millis();
          }
          break;

        case 2: //湿度页面
          if (millis() - timeStamp[2] > 1000) {
            u8g.firstPage();
            do {
              drawFrame();
              u8g.drawXBMP( 32, 0, 64, 16, u8g_menu2);
              u8g.setPrintPos(30, 42);
              u8g.print(HData, 1);
              u8g.print(" ");
              u8g.print("%");
            } while (u8g.nextPage());
            timeStamp[2] = millis();
          }
          break;

        case 3: //水平仪
          if (millis() - timeStamp[7] > 25) {
            u8g.firstPage();
            do {
              drawFrame();
              u8g.drawXBMP( 32, 0, 64, 16, u8g_menu3);
              if (rotated == 0) {
                u8g.drawCircle(64, 32, 8);
                u8g.drawDisc(A, B, 5);
                if ((64 - A) * (64 - A) + (32 - B) * (32 - B) <= 9) {
                  u8g.setPrintPos(82, 45);
                  u8g.print("OK");
                }
              }
              else {
                u8g.setPrintPos(48, 30);
                u8g.print("Not");
                u8g.setPrintPos(16, 46);
                u8g.print("Supported");
              }
            } while (u8g.nextPage());
            timeStamp[7] = millis();
          }
          break;

        case 4: //温控策略
          if (millis() - timeStamp[2] > 1000) {
            u8g.firstPage();
            do {
              drawFrame();
              u8g.drawXBMP( 32, 0, 64, 16, u8g_menu4);
              u8g.setPrintPos(14, 28);
              u8g.print("Alarm ");
              if (toalarm == 1) u8g.print(" ON");
              else u8g.print("OFF");
              u8g.setPrintPos(20, 42);
              u8g.print("Max=");
              u8g.print(RangeUp, 1);
              u8g.setPrintPos(20, 56);
              u8g.print("Min=");
              u8g.print(RangeDown, 1);
            } while (u8g.nextPage());
            timeStamp[2] = millis();
          }
          break;

        case 5: //连接状态
          if (millis() - timeStamp[2] > 1000) {
            u8g.firstPage();
            do {
              drawFrame();
              u8g.drawXBMP( 32, 0, 64, 16, u8g_menu5);
              u8g.setPrintPos(0, 31);
              u8g.print("BT:Occupied");
              u8g.setPrintPos(0, 51);
              u8g.print("WiFi:Idle");
            } while (u8g.nextPage());
            timeStamp[2] = millis();
          }
          break;
      }
      break;

    case 1:
      if (millis() - timeStamp[4] > 2000) {
        u8g.firstPage();
        do {
        } while (u8g.nextPage());
        timeStamp[4] = millis();
      }
      break;
  }
}

void COMController() {
  int i;
  String comdata = "";
  char tmp[9] = "";
  while (Serial.available())
  {
    comdata += char(Serial.read());
    delay(2);
  }
  if (comdata == "get\n") {
    Serial.println("[INFO] Here comes the data you need.");
    Serial.print("       Current Temperature (C):");
    Serial.println(TData + basictemp, 1);
    Serial.print("       Current Temperature (F):");
    Serial.println((TData + basictemp) * 9 / 5 + 32, 1);
    Serial.print("       Current Humidity (%):");
    Serial.println(HData, 1);
  }
  if (comdata == "page-\n") { //左页面
    if (stat[2] == 0) stat[2] = menu;
    else stat[2]--;
    Serial.println("[INFO] Show left page.");
  }
  if (comdata == "page+\n") { //右页面
    if (stat[2] == menu) stat[2] = 0;
    else stat[2]++;
    Serial.println("[INFO] Show right page.");
  }
  if (comdata == "noalarm\n") { //关闭警报
    toalarm = 0;
    EEPROM.write(1, toalarm);
    Serial.println("[INFO] Alarming function stops.");
  }
  if (comdata == "alarm\n") { //开启警报
    toalarm = 1;
    EEPROM.write(1, toalarm);
    Serial.println("[INFO] Alarming function starts.");
  }
  if (comdata == "nohand\n") { //关闭手势
    ishand = 0;
    Serial.println("[INFO] Hand control function stops.");
  }
  if (comdata == "hand\n") { //开启手势
    ishand = 1;
    Serial.println("[INFO] Hand control function starts.");
  }
  if (comdata == "noled\n") { //开启呼吸灯
    ledactive = 0;
    EEPROM.write(2, ledactive);
    Serial.println("[INFO] Turn off the LED.");
  }
  if (comdata == "led\n") { //关闭呼吸灯
    ledactive = 1;
    EEPROM.write(2, ledactive);
    Serial.println("[INFO] Turn on the LED.");
  }
  if (comdata == "nosleep\n") { //关闭节电模式
    nosleep = 1;
    EEPROM.write(0, nosleep);
    Serial.println("[INFO] Power saving function stops.");
  }
  if (comdata == "sleep\n") { //开启节电模式
    nosleep = 0;
    EEPROM.write(0, nosleep);
    Serial.println("[INFO] Power saving function starts.");
  }
  if (comdata[0] == 't' && comdata[1] == 'e' && comdata[2] == 'm' && comdata[3] == 'p') { //基准温度调节
    for (i = 4; i <= sizeof(comdata); i++) {
      tmp[i - 4] = comdata[i];
    }
    basictemp = atof(tmp);
    EEPROM.put(30, basictemp);
    Serial.println("[INFO] You have adjusted the temperature manually.");
    Serial.print("       ");
    Serial.print(basictemp, 1);
    Serial.println(" is added to the original data.");
  }
  if (comdata[0] == 's' && comdata[1] == 'e' && comdata[2] == 't' && comdata[3] == 'h') { //设置报警上限
    for (i = 4; i <= sizeof(comdata); i++) {
      tmp[i - 4] = comdata[i];
    }
    if (atof(tmp) >= RangeDown) {
      Serial.println("[INFO] You have set the alarming temperature value (C) to:");
      Serial.print("       Highest:");
      Serial.print(RangeUp, 1);
      Serial.print(" -> ");
      Serial.println(atof(tmp), 1);
      Serial.print("       Lowest:");
      Serial.println(RangeDown, 1);
      RangeUp = atof(tmp);
      EEPROM.put(10, RangeUp);
    } else {
      Serial.println("[WARNING] the [Highest] value must be BIGGER than the [Lowest] value!");
      Serial.print("       Highest:");
      Serial.print(RangeUp, 1);
      Serial.print(" -x> ");
      Serial.println(atof(tmp), 1);
      Serial.print("       Lowest:");
      Serial.println(RangeDown, 1);
      Serial.println("[INFO] Operation NOT finished!");
    }
  }
  if (comdata[0] == 's' && comdata[1] == 'e' && comdata[2] == 't' && comdata[3] == 'l') { //设置报警下限
    for (i = 4; i <= sizeof(comdata); i++) {
      tmp[i - 4] = comdata[i];
    }
    if (atof(tmp) <= RangeUp) {
      Serial.println("[INFO] You have set the alarming temperature value (C) to:");
      Serial.print("       Highest:");
      Serial.println(RangeUp, 1);
      Serial.print("       Lowest:");
      Serial.print(RangeDown, 1);
      Serial.print(" -> ");
      Serial.println(atof(tmp), 1);
      RangeDown = atof(tmp);
      EEPROM.put(20, RangeDown);
    } else {
      Serial.println("[WARNING] the [Lowest] value must be SMALLER than the [Highest] value!");
      Serial.print("       Highest:");
      Serial.println(RangeUp, 1);
      Serial.print("       Lowest:");
      Serial.print(RangeDown, 1);
      Serial.print(" -x> ");
      Serial.println(atof(tmp), 1);
      Serial.println("[INFO] Operation NOT finished!");
    }
  }
}

void isAlarm () {
  if (stat[0] == 0) { //非节电模式下才响
    if (stat[1] == 1 || stat[1] == 2) { //温度超出
      if ((millis() - timeStamp[3] > 100) && toalarm == 1) {
        digitalWrite(Buzzer, BuzzerSwitch);
        BuzzerSwitch = !BuzzerSwitch;
        timeStamp[3] = millis();
      }
    } else digitalWrite(Buzzer, LOW);
  } else digitalWrite(Buzzer, LOW);
}

void LEDController() {
  if (stat[0] == 0) { //非节电模式下才响
    if (stat[1] == 1 || stat[1] == 2) { //温度超出
      light = 0;
      analogWrite(LEDGreen, 0);
      analogWrite(LEDBlue, 0);
    }
  }
  if (ledactive == 1 && stat[1] == 0) {
    switch (ledlevel) {
      case 0:
        light++;
        analogWrite(LEDGreen, 0);
        analogWrite(LEDBlue, light);
        if (light >= 150) ledlevel = 1;
        break;

      case 1:
        light--;
        analogWrite(LEDGreen, 0);
        analogWrite(LEDBlue, light);
        if (light <= 0) ledlevel = 2;
        break;

      case 2:
        light++;
        analogWrite(LEDBlue, 0);
        analogWrite(LEDGreen, light);
        if (light >= 150) ledlevel = 3;
        break;

      case 3:
        light--;
        analogWrite(LEDBlue, 0);
        analogWrite(LEDGreen, light);
        if (light <= 0) ledlevel = 0;
        break;
    }
  } else {
    light = 0;
    analogWrite(LEDGreen, 0);
    analogWrite(LEDBlue, 0);
  }
}

void handScanner() {
  if ((millis() - timeStamp[6] > 50) && ishand) {
    switch (latestmove) {
      case 0:
        if ((LD <= 25) && (RD - LD >= 4)) {
          latestmove = 1;
          timeStamp[5] = millis();
        }
        if ((RD <= 25) && (LD - RD >= 4)) {
          latestmove = 1;
          timeStamp[5] = millis();
        }
        break;

      case 1:
        if ((LD >= 6) && (RD >= 6) && (LD <= 25) && (RD <= 25) && (abs(LD - RD) <= 4)) {
          latestmove = 2;
          timeStamp[5] = millis();
        }
        if ((millis() - timeStamp[5]) > 500) latestmove = 0;
        break;

      case 2:
        if ((RD <= 25) && (LD - RD >= 4)) {
          if (rotated == 0) {
            Serial.println("[INFO] RIGHT gesture detected.");
            if (stat[2] == menu) stat[2] = 0;
            else stat[2]++;
          } else {
            Serial.println("[INFO] LEFT gesture detected.");
            if (stat[2] == 0) stat[2] = menu;
            else stat[2]--;
          }
          latestmove = 0;
          timeStamp[0] = millis();
          timeStamp[6] = millis();
          timeStamp[2] = -3600;
          stat[0] = 0;
        }
        if ((LD <= 25) && (RD - LD >= 4)) {
          if (rotated == 0) {
            Serial.println("[INFO] LEFT gesture detected.");
            if (stat[2] == 0) stat[2] = menu;
            else stat[2]--;
          } else {
            Serial.println("[INFO] RIGHT gesture detected.");
            if (stat[2] == menu) stat[2] = 0;
            else stat[2]++;
          }
          latestmove = 0;
          timeStamp[0] = millis();
          timeStamp[6] = millis();
          timeStamp[2] = -3600;
          stat[0] = 0;
        }
        if ((millis() - timeStamp[5]) > 500) latestmove = 0;
        break;
    }
  }
}

void drawFrame () {
  int rad = 2; //半径
  int space = 4; //间距
  int i;
  int starting = (128 - ((menu + 1) * (2 * rad) + menu * space)) / 2;
  if (millis() - timeStamp[8] > 2000) {
    if (Angel_accZ < 0) {
      u8g.setRot180(); //反转
      rotated = 1;
    } else {
      u8g.undoRotation();
      rotated = 0;
    }
    timeStamp[8] = millis();
  }
  for (i = 0; i <= menu; i++) {
    if (stat[2] == i) u8g.drawDisc(starting + i * (2 * rad + space), 63 - rad, rad); //当前页面画实心圆
    else u8g.drawCircle(starting + i * (2 * rad + space), 63 - rad, rad); //非当前页面画空心圆
  }
}

void cleaneeprom() {
  EEPROM.write(0, 1);
  EEPROM.write(1, 1);
  EEPROM.write(2, 1);
  EEPROM.put(10, 28);
  EEPROM.put(20, 18);
  EEPROM.put(30, 0);
}

