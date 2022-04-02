#include <EtherCard.h>
#include <Arduino.h>
#include "Packet_data.h";
#include "ldbinfo.h";


#ifndef DHCPOPTIONS_H
#define DHCPOPTIONS_H

struct DHCP_DATA {
  String Option[2]={"EMPTY","EMPTY"};
};

void setLdbInfo(ldbinfo *l);
void DHCPOption(uint8_t option, const byte* data, uint8_t len);
String IPv4( String optlabel, const byte* data, uint8_t len);
String DHCP_Text(String optlabel, const byte* data, uint8_t len);
String DHCP_Time(String optlabel, const byte* data, uint8_t len);
String DHCP_Search(String optlabel, const byte* data, uint8_t len);


#endif
