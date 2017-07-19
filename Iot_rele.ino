//Se incluyen dos librerías externas: EtherCard.h para el módulo ENC28J60 y EEPROM.h para utilizar la memoria eeprom del ATMEGA 328.
#include <EtherCard.h>  
#include <EEPROM.h>
 
//Aquí se declara la dirección IP que el módulo Ethernet tendrá dentro de la red y además
//la puerta de enlace, es decir la dirección IP del router.
#define STATIC 1
#if STATIC
static byte myip[] = {10,1,1,51};
static byte gwip[] = { 10,1,1,1 };
#endif

//Esta es la dirección física (mac address del dispositivo ethernet.
static byte mymac[] = {0x74,0x69,0x69,0x2D,0x30,0x31};

byte Ethernet::buffer[1300];
BufferFiller bfill;

int LedPins[] = {3,4,5,6};

//Variable para almacenar el contenido de la EEPROM.
int leeEEprom=0;

boolean PinStatus[] = {1,2,3,4};

//Acá se declaran tres encabezados HTTP.
const char http_OK[] PROGMEM =
"HTTP/1.0 200 OK\r\n"
"Content-Type: text/html;charset=utf-8\r\n"
"Pragma: no-cache\r\n\r\n";

const char http_Found[] PROGMEM =
"HTTP/1.0 302 Found\r\n"
"Location: /\r\n\r\n";

const char http_Unauthorized[] PROGMEM =
"HTTP/1.0 401 Unauthorized\r\n"
"Content-Type: text/html\r\n\r\n"

"<h1>401 Unauthorized</h1>";

//Esta constante la declaré yo. Aquí está la página web que aparece en el navegador. Tratar de que sea lo más liviana posible. 
const char http_Body[] PROGMEM =
"<!DOCTYPE html><html><head><meta name='viewport' content='width=720, initial-scale=0.5' /><style>"
"body {"
    "background-color: #d0e4fe;"
"}"
"h1 {"
    "color: Orange;"
    "text-align: left;"
"}"
"p {"
    "font-family: 'Times New Roman';"
    "font-size: 20px;"
    "text-align: left;" 
    "color: Blue;"
"}"
"ul.a {"
    "list-style-type: circle;"
    
"}"
"</style></head><body>"
"<h1>Controle de Iluminação </h1>";


void homePage()
{
  bfill.emit_p(PSTR("$F"
    "<title>IOT</title>$F" 
        "<ul class='a'>"
    "<li style=\"font-size:30px\">Sala &nbsp;&nbsp;:  <a href=\"?ArduinoPIN1=$F\">$F</a></li>"
    "<span></span>"
    "<li style=\"font-size:30px\">Cozinha&nbsp;&nbsp;&nbsp;&nbsp;:<a href=\"?ArduinoPIN2=$F\">$F</a></li>"
    "</ul><hr>Arduino UNO"),
  http_OK, http_Body,
  PinStatus[0]?PSTR("off"):PSTR("on"),
  PinStatus[0]?PSTR("<font color=\"gren\"><b>LIGAR</b></font>"):PSTR("<font color=\"red\">DESLIGAR</font>"),
  PinStatus[1]?PSTR("off"):PSTR("on"),
  PinStatus[1]?PSTR("<font color=\"gren\"><b>LIGAR</b></font>"):PSTR("<font color=\"red\">DESLIGAR</font>"));
 // PinStatus[2]?PSTR("off"):PSTR("on"),
  //PinStatus[2]?PSTR("<font color=\"red\"><b>APAGAR</b></font>"):PSTR("<font color=\"grey\">ACENDER</font>"),
  //PinStatus[3]?PSTR("off"):PSTR("on"),
  //PinStatus[3]?PSTR("<font color=\"red\"><b>APAGAR</b></font>"):PSTR("<font color=\"grey\">ACENDER</font>"));
}


void setup()
{
  Serial.begin(9600);

  //Luego de mymac, el parámetro que le sigue corresponde a la línea CS. En este caso el CS sale por el PIN 10. 
  if (ether.begin(sizeof Ethernet::buffer, mymac,10) == 0);

#if STATIC
  ether.staticSetup(myip, gwip);
#else
  if (!ether.dhcpSetup())
  Serial.println("DHCP ha fallado. Error al asignar IP.");
#endif 

  ether.printIp("My Router IP: ", ether.myip); 

 
  ether.printIp("My SET IP: ", ether.myip); 
  for(int i = 0; i < 4 ; i++)
    {
      pinMode(LedPins[i],OUTPUT);
      //PinStatus[i]=true; 
    }
  for(int i = 0; i < 4; i++)
  { 
    leeEEprom= EEPROM.read(i);
    if(leeEEprom==1)
    {
      digitalWrite (LedPins[i],HIGH);
      PinStatus[i]=true;
    }
    if(leeEEprom==0)
    {
      digitalWrite (LedPins[i],LOW);
      PinStatus[i]=false;
    }
    
  }  
}


void loop()
{

  delay(1); 

  word len = ether.packetReceive();   
  word pos = ether.packetLoop(len);   

  if (pos) {
    bfill = ether.tcpOffset();
    char *data = (char *) Ethernet::buffer + pos;
    if (strncmp("GET /", data, 5) != 0) {
      bfill.emit_p(http_Unauthorized);
    }
   
   
    else {

      data += 5;
      if (data[0] == ' ') {       
        homePage(); 
        for (int i = 0; i <= 3; i++)
          digitalWrite(LedPins[i], leeEEprom=EEPROM.read(i));
      }

      //En esta parte se ponen en HIGH las salidas según corresponda.
      else if (strncmp("?ArduinoPIN1=on ", data, 16) == 0) {
        PinStatus[0] = true;
        EEPROM.write(0,1);        
        bfill.emit_p(http_Found);
      }
      else if (strncmp("?ArduinoPIN2=on ", data, 16) == 0) {
        PinStatus[1]= true;
        EEPROM.write(1,1);        
        bfill.emit_p(http_Found);
      }
      else if (strncmp("?ArduinoPIN3=on ", data, 16) == 0) {
        PinStatus[2] = true;        
        EEPROM.write(2,1);
        bfill.emit_p(http_Found);
      }
      else if (strncmp("?ArduinoPIN4=on ", data, 16) == 0) {
        PinStatus[3] = true;        
        EEPROM.write(3,1);
        bfill.emit_p(http_Found);
      
      }

      //En esta parte se ponen en LOW las salidas según corresponda.
      else if (strncmp("?ArduinoPIN1=off ", data, 17) == 0) {
        PinStatus[0] = false;
        EEPROM.write(0,0);        
        bfill.emit_p(http_Found);
      }
      else if (strncmp("?ArduinoPIN2=off ", data, 17) == 0) {
        PinStatus[1] = false;
        EEPROM.write(1,0);        
        bfill.emit_p(http_Found);
      }
      else if (strncmp("?ArduinoPIN3=off ", data, 17) == 0) {
        PinStatus[2] = false;
        EEPROM.write(2,0);        
        bfill.emit_p(http_Found);
      }
      else if (strncmp("?ArduinoPIN4=off ", data, 17) == 0) {
        PinStatus[3] = false;
        EEPROM.write(3,0);        
        bfill.emit_p(http_Found);
      }
      

      else {
        
        //Si la página no se encontró.
        bfill.emit_p(http_Unauthorized);
      }
    }
    ether.httpServerReply(bfill.position());    // send http response
  }
}
