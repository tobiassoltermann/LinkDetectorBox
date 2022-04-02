#include <HardwareSerial.h>
#include <driver/rtc_io.h>
#include "lldp_functions.h"
#include "cdp_functions.h"
#include "Packet_data.h"
#include <EtherCard.h>
#include "DHCPOptions.h"

#include "esp_system.h"
#include "driver/adc.h"
#include <esp_wifi.h>
#include <esp_bt.h>

#include "FS.h"
#include "SPIFFS.h"
#include "truetype_Arduino.h"
/////////////SCREEN////////////
#include <TFT_eSPI.h>
#include <SPI.h>
#include <Wire.h>
#include "WiFi.h"
// /*
#include "DEV_Config.h"
#include "EPD.h"
#include "GUI_Paint.h"
#include "ldbinfo.h"

/**/

int tft_height = 280;
int tft_width = 480;

PINFO LastCapturedPacket;
int OptionCount = 0;

byte mymac[] = {  0xCA, 0xFE, 0xC0, 0xFF, 0xEE, 0x00};
byte Ethernet::buffer[1500];
bool ENCLink;
String Protocal;
bool LogicLink = false;
bool justbooted = true;

DHCP_DATA DHCP_info[255];

static int taskCore = 0;

#include "soc/rtc_wdt.h"
#include "esp_int_wdt.h"
#include "esp_task_wdt.h"

unsigned long Interval = 1000;
unsigned long PreviousMillis = 0;
UBYTE *BlackImage;


ldbinfo l_info;

truetypeClass truetype = truetypeClass();


void setup()
{
  setLdbInfo(&l_info);
  pinMode(17,  INPUT_PULLUP);
  pinMode(16,  OUTPUT);
  digitalWrite(16, LOW);
  
  DEV_Module_Init();
  EPD_3IN7_4Gray_Init();
  EPD_3IN7_4Gray_Clear();
  DEV_Delay_ms(10);
  
  UWORD Imagesize = ((EPD_3IN7_WIDTH % 4 == 0)? (EPD_3IN7_WIDTH / 4 ): (EPD_3IN7_WIDTH / 4 + 1)) * EPD_3IN7_HEIGHT;
  if((BlackImage = (UBYTE *)malloc(Imagesize)) == NULL) {
      Serial.printf("Failed to apply for black memory...\r\n");
      while(1);
  }
  Paint_NewImage(BlackImage, EPD_3IN7_WIDTH, EPD_3IN7_HEIGHT, 270, WHITE);
  Paint_Clear(WHITE);
  Paint_SelectImage(BlackImage);
  Paint_SetScale(4);

  SPIFFS.begin(true);
  File fontFile = SPIFFS.open("/Bitter-Regular.ttf", "r");
  truetype.setFramebuffer(480, 480, 2, 0, BlackImage);
  //truetype.setFramebuffer(480, 280, 2, 0, BlackImage);

  if (!truetype.setTtfFile(fontFile)) {
    Serial.println("read ttf failed");
    return;
  }
  Paint_Clear(WHITE);

  truetype.setCharacterSize(30);
  truetype.setCharacterSpacing(0);
  truetype.setTextRotation(ROTATE_270);
  /*Paint_DrawRectangle(0, 0, 480, 45, GRAY3, DOT_PIXEL_1X1, DRAW_FILL_FULL);
  truetype.setTextColor(WHITE, WHITE);
  truetype.textDraw(10, 5, "Not connected");
  EPD_3IN7_4Gray_Display(BlackImage); */
  l_info.dirty();
  updateDisplay();

  //Pin12 is used turn on the mosfet and power the ENC and TTL convertor.
  pinMode(12, OUTPUT);
  digitalWrite(12, HIGH);

  //Disable WIFI Options to save power
  WiFi.mode(WIFI_OFF);
  esp_wifi_stop();


  if (ether.begin(sizeof (Ethernet::buffer), mymac, 5) == 0) {
  }
  else {
    ether.dhcpAddOptionCallback( 15, DHCPOption);
  }

  ENC28J60::enablePromiscuous();

}
void drawtext(String value[2]) {
  if (value[1] != "EMPTY") {
    /*tft.setTextColor(TFT_GREEN);
    tft.print(value[0] + ": ");
    tft.setTextColor(TFT_WHITE);
    //tft.print(value[1] + '\n');
    */
    Serial.println(value[0] + ":" + value[1] + '\n');

  }
}
void loop()
{
  if (millis() >= PreviousMillis + Interval) {
    PreviousMillis += Interval;
    LinkStatus(); // Uplink and DHCP
  }
  Paint_Clear(WHITE);

  int  plen = ether.packetReceive();
  byte buffcheck[1500];
  memcpy( buffcheck, Ethernet::buffer, plen );

  if (plen >= 0) {
  
    unsigned int cdp_correct = cdp_check_Packet( plen, buffcheck, plen);
    if (cdp_correct > 1) {
      Serial.println("cdp");
      l_info.l2_cdp.status = PRESENT;
      l_info.dirty();
      PINFO cdpinfo = cdp_packet_handler(buffcheck, plen);
      l_info.l2_cdp.name = cdpinfo.Name[1];
      l_info.l2_cdp.port = cdpinfo.Port[1];
      l_info.l2_cdp.speed = cdpinfo.Dup[1];
      l_info.l2_cdp.vlan = cdpinfo.VLAN[1];
      l_info.l2_cdp.ip = cdpinfo.IP[1];

      drawtext(cdpinfo.Proto);
      drawtext(cdpinfo.ProtoVer);
      drawtext(cdpinfo.Name);
      drawtext(cdpinfo.ChassisID);
      drawtext(cdpinfo.MAC);
      drawtext(cdpinfo.Port);
      drawtext(cdpinfo.Model);
      drawtext(cdpinfo.VLAN);
      drawtext(cdpinfo.IP);
      drawtext(cdpinfo.VoiceVLAN);
      drawtext(cdpinfo.Cap);
      drawtext(cdpinfo.SWver);
      drawtext(cdpinfo.PortDesc);
      drawtext(cdpinfo.Dup);
      drawtext(cdpinfo.VTP);
      drawtext(cdpinfo.TTL);



      //displayinfo(cdp_packet_handler(buffcheck, plen));
    }
    unsigned int lldp_Correct = lldp_check_Packet(plen,  buffcheck, plen);
    if (lldp_Correct > 1) {
      Serial.println("lldp");
      l_info.l2_lldp.status = PRESENT;
      l_info.dirty();
      PINFO lldpinfo = lldp_packet_handler(buffcheck, plen);
      l_info.l2_lldp.name = lldpinfo.Name[1];
      l_info.l2_lldp.port = lldpinfo.Port[1];
      l_info.l2_lldp.speed = lldpinfo.Dup[1];
      l_info.l2_lldp.vlan = lldpinfo.VLAN[1];
      l_info.l2_lldp.ip = lldpinfo.IP[1];

      drawtext(lldpinfo.Proto);
      drawtext(lldpinfo.ProtoVer);
      drawtext(lldpinfo.Name);
      drawtext(lldpinfo.ChassisID);
      drawtext(lldpinfo.MAC);
      drawtext(lldpinfo.Port);
      drawtext(lldpinfo.Model);
      drawtext(lldpinfo.VLAN);
      drawtext(lldpinfo.IP);
      drawtext(lldpinfo.VoiceVLAN);
      drawtext(lldpinfo.Cap);
      drawtext(lldpinfo.SWver);
      drawtext(lldpinfo.PortDesc);
      drawtext(lldpinfo.Dup);
      drawtext(lldpinfo.VTP);
      drawtext(lldpinfo.TTL);

      //displayinfo(lldp_packet_handler( buffcheck,  plen));
    }
  }
  updateDisplay();

  int turnoff = digitalRead(17);
  if (turnoff == LOW) {
    printf("Clear...\r\n");
    EPD_3IN7_4Gray_Clear();
    
    // Sleep & close 5V
    printf("Goto Sleep...\r\n");
    EPD_3IN7_Sleep();

    free(BlackImage);
    BlackImage = NULL;
    truetype.end();
    goToDeepSleep();
  }
  delay(10);
}

String replaceInterfaceNames(String orig) {
  String tmpResult = orig;
  tmpResult.replace("GigabitEthernet", "Gi");
  tmpResult.replace("FastEthernet", "Fa");
  tmpResult.replace("TenGigabitEthernet", "Te");
  tmpResult.replace("TwoGigabitEthernet", "Tw");
  tmpResult.replace("FiveGigabitEthernet", "Fi");
  tmpResult.replace("FortyGigabitEthernet", "Fo");
  tmpResult.replace("TwentyFiveGigE", "Twe");
  tmpResult.replace("HundredGigE", "Hu");
  tmpResult.replace("Ethernet", "Eth");
  return tmpResult;
}

void updateDisplay() {
  if (!l_info.isDirty())
    return;
  Serial.println("updateDisplay()");


  Paint_DrawRectangle(0, 0, 480, 45, GRAY3, DOT_PIXEL_1X1, DRAW_FILL_FULL);
  truetype.setCharacterSize(30);
  truetype.setTextColor(WHITE, WHITE);
  switch (l_info.l1.status) {
    case LS_DISCONN:
      truetype.textDraw(10, 5, "LinkDetector v1");
      truetype.setCharacterSize(20);
      truetype.setTextColor(WHITE, WHITE);
      Paint_DrawRectangle(226, 8, 287, 35, WHITE, DOT_PIXEL_1X1, DRAW_FILL_EMPTY);
      truetype.textDraw(235, 11, "LINK");
    break;
    case LS_CONN:
      truetype.textDraw(10, 5, "LinkDetector v1");
      truetype.setCharacterSize(20);
      truetype.setTextColor(BLACK, BLACK);
      Paint_DrawRectangle(226, 8, 287, 35, WHITE, DOT_PIXEL_1X1, DRAW_FILL_FULL);
      truetype.textDraw(235, 11, "LINK");
    break;
  }

  switch (l_info.l3.status) {
    case DHCP_WAITING:
      truetype.setCharacterSize(20);
      truetype.setTextColor(WHITE, WHITE);
      Paint_DrawRectangle(294, 8, 355, 35, WHITE, DOT_PIXEL_1X1, DRAW_FILL_EMPTY);
      truetype.textDraw(301, 11, "DHCP");
    break;

    case DHCP_FAILED:
      truetype.setCharacterSize(20);
      truetype.setTextColor(WHITE, WHITE);
      Paint_DrawRectangle(294, 8, 355, 35, WHITE, DOT_PIXEL_1X1, DRAW_FILL_EMPTY);
      truetype.textDraw(301, 11, "DHCP");
      Paint_DrawLine(294, 8, 355, 35, WHITE, DOT_PIXEL_1X1, LINE_STYLE_DOTTED);
      Paint_DrawLine(355, 8, 294, 35, WHITE, DOT_PIXEL_1X1, LINE_STYLE_DOTTED);
    break;
    case DHCP_SUCCESS:
      truetype.setCharacterSize(20);
      truetype.setTextColor(BLACK, BLACK);
      Paint_DrawRectangle(294, 8, 355, 35, WHITE, DOT_PIXEL_1X1, DRAW_FILL_FULL);
      truetype.textDraw(301, 11, "DHCP");

      truetype.setCharacterSize(20);
      truetype.setTextColor(GRAY3, GRAY3);
      truetype.textDraw(10, 60, "IP:");
      truetype.textDraw(10, 60+1*(22+2), "NM:");
      //truetype.textDraw(10, 60+2*(22+2), "GW:");
      //truetype.textDraw(10, 60+3*(22+2), "NS:");
      truetype.textDraw(240, 60, "GW:");
      truetype.textDraw(240, 60+1*(22+2), "NS:");


      truetype.setTextColor(BLACK, BLACK);

      truetype.setCharacterSize(30);
      truetype.textDraw(60, 53, l_info.l3.ip);
      truetype.setCharacterSize(22);
      truetype.textDraw(60, 60+1*(22+2), l_info.l3.netMask + " (/"+l_info.l3.netMaskCIDR()+")");
//      truetype.textDraw(60, 59+2*(22+2), l_info.l3.gw);
//      truetype.textDraw(60, 59+3*(22+2), l_info.l3.ns);
      truetype.textDraw(290, 60, l_info.l3.gw);
      truetype.textDraw(290, 60+1*(22+2), l_info.l3.ns);           
    break;
  }
  switch (l_info.l2_lldp.status) {
    case WAITING:
      truetype.setCharacterSize(20);
      truetype.setTextColor(WHITE, WHITE);
      Paint_DrawRectangle(362, 8, 415, 35, WHITE, DOT_PIXEL_1X1, DRAW_FILL_EMPTY);
      truetype.textDraw(370, 11, "LLDP");
    break;
    case PRESENT:
      truetype.setCharacterSize(20);
      truetype.setTextColor(BLACK, BLACK);
      Paint_DrawRectangle(362, 8, 415, 35, WHITE, DOT_PIXEL_1X1, DRAW_FILL_FULL);
      truetype.textDraw(370, 11, "LLDP");

      truetype.setCharacterSize(20);
      truetype.textDraw(80, 135 + 1*(33+4), l_info.l2_lldp.name);
      truetype.textDraw(80, 135 + 2*(33+4), replaceInterfaceNames(l_info.l2_cdp.port));
      truetype.textDraw(80, 135 + 3*(33+4), l_info.l2_lldp.vlan);
      truetype.textDraw(345, 135 + 3*(33+4), l_info.l2_lldp.speed);
      truetype.textDraw(310, 135 + 1*(33+4), l_info.l2_lldp.ip);
    break;
  }
  switch (l_info.l2_cdp.status) {
    case WAITING:
      truetype.setCharacterSize(20);
      truetype.setTextColor(WHITE, WHITE);
      Paint_DrawRectangle(425, 8, 470, 35, WHITE, DOT_PIXEL_1X1, DRAW_FILL_EMPTY);
      truetype.textDraw(431, 11, "CDP");
    break;
    case PRESENT:
      truetype.setCharacterSize(20);
      truetype.setTextColor(BLACK, BLACK);
      Paint_DrawRectangle(425, 8, 470, 35, WHITE, DOT_PIXEL_1X1, DRAW_FILL_FULL);
      truetype.textDraw(431, 11, "CDP");


      truetype.setCharacterSize(20);
      truetype.textDraw(80, 118 + 1*(33+4), l_info.l2_cdp.name);
      truetype.textDraw(80, 118 + 2*(33+4), replaceInterfaceNames(l_info.l2_cdp.port));
      truetype.textDraw(80, 118 + 3*(33+4), l_info.l2_cdp.vlan);
      truetype.textDraw(345, 118 + 3*(33+4), l_info.l2_cdp.speed);
      truetype.textDraw(310, 118 + 1*(33+4), l_info.l2_cdp.ip);
    break;
  }

  if (l_info.l2_lldp.status == PRESENT || l_info.l2_cdp.status == PRESENT) {
   
    truetype.setTextColor(GRAY3, GRAY3);
    truetype.setCharacterSize(20);
    truetype.textDraw(10, 125, "Peer info:");
    truetype.textDraw(10, 127 + 1*(33+4) , "Name:");
    truetype.textDraw(10, 127 + 2*(33+4) , "Port:");
    truetype.textDraw(10, 127 + 3*(33+4) , "VLAN:");
    truetype.textDraw(290, 127 + 3*(33+4) , "Mode:");
    truetype.textDraw(290, 127 + 1*(33+4) , "IP:");
    truetype.setTextColor(BLACK, BLACK);

  } else {
    truetype.setTextColor(GRAY1, GRAY1);
    truetype.setCharacterSize(20);
    truetype.textDraw(10, 125, "No peer info");
  }

  Serial.println("display");
  EPD_3IN7_4Gray_Display(BlackImage); 
  l_info.clean();
}

void LinkStatus()
{
  bool LinkStat = ENC28J60::isLinkUp();
  l1info oldStatus;
  oldStatus.status = l_info.l1.status;

  if (ENCLink != LinkStat ) {// || justbooted == true) {
    justbooted = false;
    ENCLink = LinkStat;
    if (LinkStat == true ) {
      if (oldStatus.status != LS_CONN) {
        Serial.println("LinkStat: true");
        l_info.l1.status = LS_CONN;
        l_info.l3.status = DHCP_WAITING;
        l_info.dirty();
        updateDisplay();
      }
      DHCP();

    }
    if (LinkStat == false ) {
      if (oldStatus.status != LS_DISCONN) {
  
        Serial.println("LinkStat: false");
        l_info.l1.status = LS_DISCONN;
        l_info.l2_cdp.status = WAITING;
        l_info.l2_lldp.status = WAITING;
        l_info.l3.status = DHCP_WAITING;
        l_info.dirty();
        updateDisplay();
      }
    }
  }
}

void DHCP() {

  // Reset Array to defaults.
  for ( int j = 0; j < 255; ++j) {
    DHCP_info[j].Option[0] = "EMPTY";
    DHCP_info[j].Option[1] = "EMPTY";
  }
  //Check for DHCP
  if (!ether.dhcpSetup())
  {
    //If Fails
    l_info.l3.status = DHCP_FAILED;
    l_info.dirty();
    updateDisplay();
  }
  else
  {
    //displayDHCP();
    l_info.l3.status = DHCP_SUCCESS;
    String ipaddy;
    for (unsigned int j = 0; j < 4; ++j) {
      ipaddy  += String(ether.myip[j]);
      if (j < 3) {
        ipaddy += ".";
      }
    }
    l_info.l3.ip = ipaddy;
    l_info.dirty();
    updateDisplay();
  }
}


void goToDeepSleep()
{
  //int r = digitalRead(TFT_BL);
/*  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  tft.setTextDatum(MC_DATUM);
  tft.drawString(String("Press again to wake up"),  tft.width() / 2, tft.height() / 2);*/

  //digitalWrite(TFT_BL, !r);

/*  tft.writecommand(TFT_DISPOFF);
  tft.writecommand(TFT_SLPIN);

  btStop();
  adc_power_off();
  */
  esp_wifi_stop();
  esp_bt_controller_disable();

  //After using light sleep, you need to disable timer wake, because here use external IO port to wake up
  esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_TIMER);
//  esp_sleep_enable_ext0_wakeup(GPIO_NUM_35, 0);
  delay(200);
  esp_deep_sleep_start();
}
