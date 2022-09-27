/**
   StreamHTTPClient.ino

    Created on: 24.05.2015

*/

#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <FS.h>
#include <EEPROM.h>

//-----------------------------------------------------------------------------------------------------//
//#define SERIAL_OUT

#ifdef SERIAL_OUT

#define SOH    'a'
#define STX   'b'
#define ACK   'd'
#define NAK   'e'
#define EOT   'f'
#define ETB   'g'
#define CANCEL  'h'
#define X_C   'C'

#else

#define SOH    0x01
#define STX   0x02
#define ACK   0x06
#define NAK   0x15
#define EOT   0x04
#define ETB   0x17
#define CANCEL  0x18
#define X_C   0x43

#endif
#define PKT_SIZE_1K 1024
//----------------------------------------------------------------------------------------//

char *ssid=">>DKC<<", *password="888121212";
//char *ssid="EMOB_048", *password="mithun1234";

WiFiClient client;
HTTPClient http; //must be declared after WiFiClient for correct destruction order, because used by http.begin(client,...)
uint8_t data[1024];

void setup() {
  pinMode(0,INPUT);
  pinMode(LED_BUILTIN,OUTPUT);
  digitalWrite(LED_BUILTIN,HIGH);
  //Serial1.begin(115200);  //for debug messages
  Serial.begin(115200);   //for uart xmodem ftp

  Serial.println();
  Serial.println();
  Serial.println();
  
  if(!SPIFFS.begin()) Serial.printf("SPIFFS not found\n\r");
  else Serial.printf("SPIFFS found\n\r");
  
  Serial.printf("Conecting to wifi");
  WiFi.begin(ssid,password);
  
  while (WiFi.status() != WL_CONNECTED) {
    Serial.printf(".");
    delay(500);
  }
  Serial.println("\n\rConnected to "+WiFi.SSID()+" Use IP address: "+WiFi.localIP().toString());
  EEPROM.begin(512);
}

void loop() {
    File current,newfile;
    int sizeoffilenew, sizeoffilecurrent, httpCode, eeprom_add=0;
    
    http.begin(client, "http://192.168.43.191/share/BL_User_1.bin");
    http.addHeader("Content-Type","application/octet-stream");
    httpCode = http.GET();
    Serial.printf("http.get = %d\n\r",httpCode);
    sizeoffilenew = http.getSize();
    
    if (httpCode == HTTP_CODE_OK) {
      current = SPIFFS.open("/current.bin","r+");
      if(!current) Serial.printf("Error in opening current file\n\r");
      sizeoffilecurrent = current.size();
      Serial.printf("size of new file=%d\n\r",sizeoffilenew);
      Serial.printf("size of current file=%d\n\r",sizeoffilecurrent);
      
      if(sizeoffilenew!=sizeoffilecurrent)
      { 
        current = SPIFFS.open("/current.bin","w+");
        write_file(&client,current);
        EEPROM.write(eeprom_add,1);  // update flag
        Serial.printf("Size is different so instantly downloaded to current file\n\r");  
      }

      else
      {
         newfile = SPIFFS.open("/new.bin","w+");
         write_file(&client,newfile);
       
        if(file_cmp(current,newfile)!=0)
        {
          replace_file(current,newfile);
          EEPROM.write(eeprom_add,1);  // update flag
          Serial.printf("file same size but different contents so file replaced!!!\n\r");
        }
        else  
        {
          EEPROM.write(eeprom_add,0);  // update flag
          Serial.printf("Same file so not replaced\n\r");
        }
        newfile.close();
        Serial.printf("newfile deleted: %d\n\r",SPIFFS.remove("/new.bin"));
      }
  
    if(EEPROM.read(eeprom_add))   Serial.printf("Update available\n\r");
    else    Serial.printf("Update not available\n\r");
    }
    http.end();
    current.seek(0,SeekSet);
    digitalWrite(LED_BUILTIN,LOW);
    while(digitalRead(0) != LOW)    wdt_reset();
    send_file(current);
    digitalWrite(LED_BUILTIN,HIGH);
    current.close();
}

int file_cmp(File current, File newfile)
{
  current.seek(0,SeekSet);
  newfile.seek(0,SeekSet);
  union 
  {
    uint8_t var8[4];
    uint32_t var32;
  } buff_curr, buff_new;
  int cmp_flag = 0;
  while(current.read(buff_curr.var8,sizeof(buff_curr.var8)) && newfile.read(buff_new.var8,sizeof(buff_new.var8)))
    if(buff_curr.var32!=buff_new.var32)
    {  
      cmp_flag=1;
      break;
    }
  return cmp_flag;
}

void replace_file(File current, File newfile)
{
  current.seek(0,SeekSet);
  newfile.seek(0,SeekSet);
  uint8_t buff[4];
  while(newfile.read(buff,sizeof(buff)))
    current.write(buff,sizeof(buff));  
}

void write_file(WiFiClient *stream, File file)
{
  uint8_t buff[4];
  while(stream->readBytes(buff, sizeof(buff)))
          file.write(buff,sizeof(buff));
}

//---------------------------------------------------------------------------------------------------------//
uint16_t xmodem_calcrc(unsigned char *ptr, int count)
{
    union
    {
      uint16_t val16;
      uint8_t val8[2];
    }crc;
    
    uint8_t i,temp;

    crc.val16 = 0;
    while (--count >= 0)
    {
        crc.val16 = crc.val16 ^ *ptr++ << 8;
        i = 8;
        do
        {
            if (crc.val16 & 0x8000)
                crc.val16 = crc.val16 << 1 ^ 0x1021;
            else
                crc.val16 = crc.val16 << 1;
        } while(--i);
    }
    temp = crc.val8[1];
    crc.val8[1] = crc.val8[0];
    crc.val8[0] = temp;
    return (crc.val16);
}

/*unsigned char * get_packet(int packet_num, File current){
  static int last_pack=0;
  last_pack=(PKT_SIZE*(packet_num-1));
  if(last_pack < sizeof(buf))
    return data+(PKT_SIZE*(packet_num-1));
  else
    return NULL;
}*/

uint8_t get_packet_1k(File current,int num)
{
  int read_size,i,EOT_flag=0;
  current.seek((num-1)*PKT_SIZE_1K,SeekSet);
  read_size = current.read(data,sizeof(data));
  if(read_size!=PKT_SIZE_1K)
  {
     EOT_flag = 1;
     for(i=read_size;i<PKT_SIZE_1K;i++) 
        data[i] = 0xFF;
  }
  return EOT_flag;
}

void xmodem_wait_for_send(){
  unsigned char ch=0;
  while(ch!=X_C)
  {
    ch=Serial.read();
    wdt_reset();
  }
}

void send_file(File current)
{
  Serial.print("1");
//  delay(1000);
  uint8_t _packet_num=1,_header, EOT_flag=0,response=0;
  uint16_t crc;
  int _size;
  
  if(_packet_num==1)
    xmodem_wait_for_send();  
  
  do{
    //start txn
    _size=PKT_SIZE_1K;
    _header=STX;
    
    EOT_flag=get_packet_1k(current,_packet_num);

    crc = xmodem_calcrc(data, _size);

#ifndef SERIAL_OUT    
    //send header
    Serial.write(_header);
    delay(1000);
    //send packet number;
    Serial.write(_packet_num);
    //send !(packet number)
    Serial.write(!_packet_num);
    //send data
    Serial.write(data,_size);
    Serial.write(crc);

#else
    
  Serial.printf("%c %d %d ",_header,_packet_num,~_packet_num);
  for(int i=0;i<1024;i++)
  Serial.printf("%02X ",data[i]);
  Serial.printf("%04X\n\r",crc);
  Serial.println("Enter ACK");
#endif          //---------------------------------------------------------------------------------

    while(response!=ACK){
      response=Serial.read();
      wdt_reset();
    }  
    if(response==ACK)
    {
      _packet_num++;
      response=0;     
    }
    else if(response==CANCEL) break;
    if(EOT_flag==1)  _header=EOT;
  } while(EOT_flag==0);

  //EOT of file//
  uint8_t end_tx=EOT;
#ifndef SERIAL_OUT  
  Serial.write(end_tx);
#endif
  Serial.println("transmit completed!!!!");
}
//--------------------------------------------------------------------------------------------------------//
