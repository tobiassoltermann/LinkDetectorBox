#include <Arduino.h>
#include "DHCPOptions.h";
extern DHCP_DATA DHCP_info[255];
extern int OptionCount;
#define DHCP_INFINITE_LEASE  0xffffffff

ldbinfo *_l_info;

void setLdbInfo(ldbinfo *l) {
  _l_info = l;
}
void printOption(String name, String value) {
  Serial.println(name + ":" + value + '\n');
}
void DHCPOption(uint8_t option, const byte* data, uint8_t len) {
  //likely to be an IPv4 address/Subnet
  switch (option) {
    case 1:
      printOption("MASK", _l_info->l3.netMask);
      _l_info->l3.netMask = IPv4("MASK", data, len);
      break;

    case 3:
      printOption("GW", _l_info->l3.gw);
      _l_info->l3.gw = IPv4("GW", data, len);
      break;

    case 4:
      if (len == 4) {
        _l_info->l3.time = IPv4("TIME:", data, len);
      }
      else {
        _l_info->l3.time = DHCP_Text("TIME", data, len - 4);
      }
      printOption("TIME", _l_info->l3.time);
      break;

    case 6:
      if (len > 0) {
        int a = 0;
        /*for ( int i = 0; i <   (len / 4) ; ++i ) {
          _l_info->l3.ns = IPv4("DNS" +  String(i + 1), data + (i * 4) , 4 );
          a = a + 4;
          if (((i * 4) + 4) != len) {
            OptionCount++;
          }
        }*/
        _l_info->l3.ns = IPv4("DNS1", data, 4 );
        printOption("DNS1", _l_info->l3.ns);
      }
      break;

    case 15:
      _l_info->l3.domain = DHCP_Text("DOMAIN", data, len);
      printOption("DOMAIN", _l_info->l3.domain);
      break;

    case 42:
      if (len == 4) {
        _l_info->l3.ntp = IPv4("NTP:", data, len);
      }
      else {
        _l_info->l3.ntp = DHCP_Text("NTP", data, len - 4);
      }
      printOption("NTP", _l_info->l3.ntp);
      break;

    case 44:
      _l_info->l3.netbios = IPv4("NETBIOS", data, len);
      printOption("NETBIOS", _l_info->l3.netbios);
      break;

    case 51:
      _l_info->l3.lease = DHCP_Time("LEASE", data, len);
      printOption("LEASE", _l_info->l3.lease);
      break;

    case 54:
      _l_info->l3.dhcp = IPv4("DHCP", data, len);
      printOption("DHCP", _l_info->l3.dhcp);
      break;

    case 53:
      //skip DHCP_Text("DHCP Msg Type ", data, len);
      break;

    case 58:
      // Skip DHCP_Time("RENEWAL", data, len);
      break;

    case 59:
      // Skip DHCP_Time("REBIND", data, len);
      break;

    case 66:
      _l_info->l3.tftp = DHCP_Text("TFTP", data, len);
      printOption("TFTP", _l_info->l3.tftp);
      break;

    case 67:
      _l_info->l3.pxefile = DHCP_Text("PXE FILE", data, len);
      printOption("PXE FILE", _l_info->l3.pxefile);
      break;

    case 77:
      // Skip option
      break;

    case 119:
      _l_info->l3.searchdomain = DHCP_Search("SEARCH DOMAIN", data, len);
      printOption("SEARCH DOMAIN", _l_info->l3.searchdomain);
      break;

    case 255:
      // Skip option
      break;

    default: {
        byte i;
        char temp [len + 1] ;
        int a = 0;
        String temp1;
        for ( i = 0; i < ( len ); ++i , ++a) {
          temp[a] = data[i];
        }
        temp[len] = '\0';
        temp1  = temp;
        Serial.println("OPT: " + String(option) + " LEN:" +  String(len) + " Data:" + temp1 + '\n');
      }
  }
  OptionCount++;
}

String IPv4( String optlabel, const byte* data, uint8_t len) {
  byte address[4];
  String temp;
  for (unsigned int j = 0; j < len; ++j) {
    address[j] = data[j];
    temp += address[j] ;
    if (j < 3) {
      temp += ".";
    }
  }
  return temp;
/*  Serial.println(optlabel + ":" + temp);
  DHCP_info[OptionCount].Option[0] = optlabel;
  DHCP_info[OptionCount].Option[1] = temp;
*/
}

String DHCP_Text(String optlabel, const byte* data, uint8_t len) {
  byte i;
  char temp [len + 1] ;
  int a = 0;
  String temp1;
  for ( i = 0; i < ( len ); ++i , ++a) {
    temp[a] = data[i];
  }
  temp[len] = '\0';
  temp1  = temp;
  return temp1;
}

String DHCP_Search(String optlabel, const byte* data, uint8_t len) {
  char temp [len ] ;
  int a = 0;
  int b = 0;
  String temp1;

  unsigned int DataIndex = 0;
  while (DataIndex < len ) {
    unsigned int Searchlen = data[DataIndex];
    if (Searchlen == 0) {
      temp1 += "\n";
      DataIndex++;
    }
    else {
      DataIndex++;
      for (int  i = 0; i < Searchlen; ++i , ++a) {
        temp[a] = data[i + DataIndex ];
      }
      temp[Searchlen] = '\0';
      temp1 += temp;
      if ((DataIndex + Searchlen) != len && data[DataIndex + Searchlen] != 0) {
        temp1 += ".";
      }
      DataIndex += Searchlen;
      a = 0;
    }
  }
  /*Serial.println(optlabel + ":" + temp1);
  DHCP_info[OptionCount].Option[0] = optlabel;
  DHCP_info[OptionCount].Option[1] = temp1;*/
  return temp1;
}

String DHCP_Time(String optlabel, const byte * data, uint8_t len) {
  unsigned long num = 0;
  int temp1;
  for (unsigned int i = 0; i < len; ++i) {
    num <<= 8;
    num += data[i];
  }
  temp1 =  (int)num;
  temp1 = ((temp1 / 60) / 60);
/*  Serial.println(optlabel + ":" + String(temp1) + "hrs");
  DHCP_info[OptionCount].Option[0] = optlabel;
  DHCP_info[OptionCount].Option[1] = String(temp1) + "hrs";*/
  return String(temp1) + "hrs";
}
