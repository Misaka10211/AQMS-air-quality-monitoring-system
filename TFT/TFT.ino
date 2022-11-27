#include <LCDWIKI_GUI.h> //Core graphics library
#include <LCDWIKI_SPI.h> //Hardware-specific library
#include <LCDWIKI_TOUCH.h> //touch screen library
//TFT*************************************************************
//paramters define
#define MODEL ILI9341
#define CS   A5
#define CD   A3
#define RST  A4
#define LED  -1   //if you don't need to control the LED pin,you should set it to -1 and set it to 3.3V
LCDWIKI_SPI my_lcd(MODEL,CS,CD,RST,LED);  //model, cs, dc, reset, led
                             /*  r     g    b */
#define BLACK        0x0000  /*   0,   0,   0 */
#define BLUE         0x001F  /*   0,   0, 255 */
#define RED          0xF800  /* 255,   0,   0 */
#define GREEN        0x07E0  /*   0, 255,   0 */
#define CYAN         0x07FF  /*   0, 255, 255 */
#define MAGENTA      0xF81F  /* 255,   0, 255 */
#define YELLOW       0xFFE0  /* 255, 255,   0 */
#define WHITE        0xFFFF  /* 255, 255, 255 */
#define NAVY         0x000F  /*   0,   0, 128 */
#define DARKGREEN    0x03E0  /*   0, 128,   0 */
#define DARKCYAN     0x03EF  /*   0, 128, 128 */
#define MAROON       0x7800  /* 128,   0,   0 */
#define PURPLE       0x780F  /* 128,   0, 128 */
#define OLIVE        0x7BE0  /* 128, 128,   0 */
#define LIGHTGREY    0xC618  /* 192, 192, 192 */
#define DARKGREY     0x7BEF  /* 128, 128, 128 */
#define ORANGE       0xFD20  /* 255, 165,   0 */
#define GREENYELLOW  0xAFE5  /* 173, 255,  47 */
#define PINK         0xF81F  /* 255,   0, 255 */

//DHT-11*************************************************************
#include "DHT.h" //加载DHT11的库
#define DHTTYPE DHT11 // 定义传感器类似 DHT11
#define DHTPIN 7 
DHT dht(DHTPIN, DHTTYPE);//声明 dht 函数

//SGP-30*************************************************************
#include "SGP30.h"
SGP mySGP30;
u16 CO2Data,TVOCData;//定义CO2浓度变量与TVOC浓度变量
u32 sgp30_dat;
/*
SCL -> A1
SDA -> A2
*/

//PM2.5**************************************************************
#define dustPin A0
#define ledPower 2
float dustVal=0; 
int delayTime=280;
int delayTime2=40;
int offTime=9680;

void show_string(uint8_t *str,int16_t x,int16_t y,uint8_t csize,uint16_t fc, uint16_t bc,boolean mode)
{
    my_lcd.Set_Text_Mode(mode);
    my_lcd.Set_Text_Size(csize);
    my_lcd.Set_Text_colour(fc);
    my_lcd.Set_Text_Back_colour(bc);
    my_lcd.Print_String(str,x,y);
}

float t=00.00;
float h=00.00;
void update_DHT(){
  t = dht.readTemperature();//读取摄氏度
  h = dht.readHumidity()+9;//读取湿度,【+系统误差9】
  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
  }
  my_lcd.Set_Draw_color(BLACK);
  my_lcd.Fill_Rectangle(94, 213, 185, 235);
  my_lcd.Set_Text_Size(3);
  my_lcd.Print_Number_Float(t, 2, 95, 214, '.', 0, ' '); 
  
  my_lcd.Set_Draw_color(BLACK);
  my_lcd.Fill_Rectangle(94, 276, 185, 298);
  my_lcd.Set_Text_Size(3);
  my_lcd.Print_Number_Float(h, 2, 95, 277, '.', 0, ' '); 
}

int SGP_flag;// 指示SGP30是否完成初始化
void SGP30_INIT(){
  SGP_flag=0;
  mySGP30.SGP30_Init();
  mySGP30.SGP30_Write(0x20,0x08);
  sgp30_dat = mySGP30.SGP30_Read();//读取SGP30的值
  CO2Data = (sgp30_dat & 0xffff0000) >> 16;
  TVOCData = sgp30_dat & 0x0000ffff; 
  //SGP30模块开机需要一定时间初始化，在初始化阶段读取的CO2浓度为400ppm，TVOC为0ppd且恒定不变，因此上电后每隔一段时间读取一次
  //SGP30模块的值，如果CO2浓度为400ppm，TVOC为0ppd，发送“正在检测中...”，直到SGP30模块初始化完成。
  //初始化完成后刚开始读出数据会波动比较大，属于正常现象，一段时间后会逐渐趋于稳定。
  //气体类传感器比较容易受环境影响，测量数据出现波动是正常的，可自行添加滤波函数。
}

void update_SGP30(){
  if(CO2Data == 400 && TVOCData == 0 && SGP_flag==0)
  {
    mySGP30.SGP30_Write(0x20,0x08);
    sgp30_dat = mySGP30.SGP30_Read();//读取SGP30的值
    CO2Data = (sgp30_dat & 0xffff0000) >> 16;//取出CO2浓度值
    TVOCData = sgp30_dat & 0x0000ffff;       //取出TVOC值
    Serial.println("正在检测中...");
    my_lcd.Set_Draw_color(BLACK);
    my_lcd.Fill_Rectangle(85, 11, 236, 53);
    my_lcd.Fill_Rectangle(85, 138, 236, 180);
    show_string("INIT",85,19,4,LIGHTGREY, BLACK,1);
    show_string("INIT",85,147,4,LIGHTGREY, BLACK,1);
    SGP_flag=1;
  }
  else{
    mySGP30.SGP30_Write(0x20,0x08);
    sgp30_dat = mySGP30.SGP30_Read();//读取SGP30的值
    CO2Data = (sgp30_dat & 0xffff0000) >> 16;//取出CO2浓度值
    TVOCData = sgp30_dat & 0x0000ffff;       //取出TVOC值
    Serial.print("CO2:");
    Serial.print(CO2Data,DEC);
    Serial.println("ppm");
    Serial.print("TVOC:");
    Serial.print(TVOCData,DEC);
    Serial.println("ppd");
    
    my_lcd.Set_Draw_color(BLACK);
    my_lcd.Set_Text_Size(4);
    //【CO2】
    if(CO2Data<600){my_lcd.Set_Text_colour(GREEN);}
    else if(CO2Data>=600 && CO2Data<1000){my_lcd.Set_Text_colour(YELLOW);}
    else{my_lcd.Set_Text_colour(RED);}
    my_lcd.Fill_Rectangle(85, 11, 236, 53);
    my_lcd.Print_Number_Int(CO2Data,87, 19, 2,' ',10); 

    //【TVOC】
    if(TVOCData<250){my_lcd.Set_Text_colour(GREEN);}
    else if(TVOCData>=250 && TVOCData<500){my_lcd.Set_Text_colour(YELLOW);}
    else{my_lcd.Set_Text_colour(RED);}
    my_lcd.Fill_Rectangle(85, 138, 236, 180);
    my_lcd.Print_Number_Int(TVOCData,87, 146, 2,' ',10); 

    my_lcd.Set_Text_colour(LIGHTGREY);
  }
}

float prev[10];
int PM25_flag;// 指示PM2.5是否完成初始化
void PM25_INIT(){
  PM25_flag=0;
  pinMode(ledPower,OUTPUT);
  pinMode(dustPin, INPUT);
  for(int i=0;i<10;i++){
    prev[i]=0;
  }
}

void update_PM25(){
  digitalWrite(ledPower,LOW); 
  
  delayMicroseconds(delayTime);
  dustVal=analogRead(dustPin); 
  
  delayMicroseconds(delayTime2);
  digitalWrite(ledPower,HIGH); 
  
  delayMicroseconds(offTime);
  //float PM25Val = (float(dustVal/ 1023) - 0.0356) * 120000 * 0.035;
  float PM25Val = (float(dustVal*(5.0/1023) - 0.46)) * 24000 /29;
  float calVoltage = dustVal*(5.0/1023);
  //float PM25Val = (5000*calVoltage-2400)/29;
  //减小误差，取近3次均值作为输出
  if (PM25Val<0){
    PM25Val = 0.00;
  }
  Serial.print("V=");
  Serial.print(calVoltage);
  Serial.print(" P2=");
  int x=1;
  Serial.print(prev[x]);
  Serial.print(" P1=");
  x=0;
  Serial.print(prev[x]);
  Serial.print(" PM25=");
  Serial.print(PM25Val);
  float total=0.0;
  int ValidData=0;
  for(int i=0;i<10;i++){
    if(prev[i]!=0){
      total+=prev[i];
      ValidData+=1;
    }
  }
  ValidData+=1;
  PM25Val=(total+PM25Val)/ValidData;
  Serial.print(" PM25*=");
  Serial.println(PM25Val);
  prev[0]=PM25Val;
  Serial.print(" P1=");
  Serial.println(prev[0]);
  for(int i=9;i>0;i--){
    Serial.println(prev[i]);
    prev[i]=prev[i-1];
  }
  /*Serial.print(" P2=");
  Serial.print(prev[1]);
  Serial.print(" P1=");
  Serial.println(prev[0]);*/
  if(PM25_flag==0){
    show_string("INIT",85,83,4,LIGHTGREY, BLACK,1);//PM2.5
    PM25_flag=1;
  }
  else{
    if(PM25Val<75){my_lcd.Set_Text_colour(GREEN);}
    else if(PM25Val>=75 && PM25Val<150){my_lcd.Set_Text_colour(YELLOW);}
    else if(PM25Val>=150 && PM25Val<250){my_lcd.Set_Text_colour(ORANGE);}
    else{my_lcd.Set_Text_colour(RED);}
    my_lcd.Set_Draw_color(BLACK);
    my_lcd.Fill_Rectangle(85, 75, 236, 117);
    my_lcd.Print_Number_Float(PM25Val, 2, 87, 80, '.', 0, ' '); 
  
    my_lcd.Set_Text_colour(LIGHTGREY);
  }
}

void show_menu(void)
{  
   my_lcd.Set_Draw_color(LIGHTGREY);
   my_lcd.Fill_Rectangle(0, 0, my_lcd.Get_Display_Width(), 2);
   my_lcd.Fill_Rectangle(0, 63, my_lcd.Get_Display_Width(), 65);
   my_lcd.Fill_Rectangle(0, 127, my_lcd.Get_Display_Width(), 129);
   my_lcd.Fill_Rectangle(0, 191, my_lcd.Get_Display_Width(), 193);
   my_lcd.Fill_Rectangle(0, 255, my_lcd.Get_Display_Width(), 257);
   my_lcd.Fill_Rectangle(0, 318, my_lcd.Get_Display_Width(), 320);

   my_lcd.Fill_Rectangle(0, 0, 2, my_lcd.Get_Display_Height());
   my_lcd.Fill_Rectangle(80, 0, 82, my_lcd.Get_Display_Height());
   my_lcd.Fill_Rectangle(237, 0, 239, my_lcd.Get_Display_Height());
   
   show_string("CO",10,10,4,LIGHTGREY, BLACK,1); 
   show_string("2",60,21,3,LIGHTGREY, BLACK,1); 
   show_string("ppm",25,42,2,LIGHTGREY, BLACK,1); 
   
   //my_lcd.Set_Text_Size(4);
   //my_lcd.Print_Number_Float(1234.56, 2, 87, 15, '.', 0, ' ');  
   
   show_string("PM",7,75,3,LIGHTGREY, BLACK,1); 
   show_string("2.5",43,82,2,LIGHTGREY, BLACK,1); 
   show_string("ug/m",20,105,2,LIGHTGREY, BLACK,1);
   show_string("3",68,103,1,LIGHTGREY, BLACK,1);

   //my_lcd.Set_Text_Size(4);
   //my_lcd.Print_Number_Float(123.6, 2, 87, 80, '.', 0, ' '); 

   show_string("TVOC",7,139,3,LIGHTGREY, BLACK,1); 
   show_string("mg/m",20,167,2,LIGHTGREY, BLACK,1);
   show_string("3",68,165,1,LIGHTGREY, BLACK,1);

   //my_lcd.Set_Text_Size(4);
   //my_lcd.Print_Number_Float(0.065, 2, 87, 144, '.', 0, ' '); 

   show_string("TEMP",7,214,3,LIGHTGREY, BLACK,1); 
   my_lcd.Draw_Circle(192, 210, 5);
   show_string("C",204,210,5,LIGHTGREY, BLACK,1); 

   my_lcd.Set_Text_Size(3);
   my_lcd.Print_Number_Float(t, 2, 95, 214, '.', 0, ' '); 
   
   show_string("HUMI",9,277,3,LIGHTGREY, BLACK,1);
   my_lcd.Draw_Circle(204, 276, 5);
   my_lcd.Draw_Circle(219, 301, 5);
   show_string("/",199,272,5,LIGHTGREY, BLACK,1);

   my_lcd.Set_Text_Size(3);
   my_lcd.Print_Number_Float(h, 2, 95, 277, '.', 0, ' '); 
}

void setup() {
  // put your setup code here, to run once:
  my_lcd.Init_LCD();
  my_lcd.Set_Rotation(2); 
  my_lcd.Fill_Screen(BLACK);
  
  dht.begin(); //启动传感器
  show_menu();
  SGP30_INIT();
  PM25_INIT();
  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  update_SGP30();
  update_PM25();
  update_DHT();
  delay(2000);
}
