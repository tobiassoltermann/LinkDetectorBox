#ifndef LDBINFO_H
#define LDBINFO_H

typedef enum {
  LS_DISCONN = 0,
  LS_CONN,
  LS_WAIT,
} L1STATUS;



typedef enum {
  WAITING = 0,
  PRESENT,
} L2STATUS;

struct l1info {
  bool d;
  L1STATUS status = LS_DISCONN;

  bool operator==(const l1info& o) const {
      return
        status == o.status
      ;
  }
  void clean() {
    d = false;
  }
};

struct l2info {
  L2STATUS status = WAITING;
  String name;
  String port;
  String speed;
  String vlan;
  String ip;
  bool operator==(const l2info& o) const {
    return
      status == o.status
      && name == o.name
      && port == o.port
      && speed == o.speed
      && vlan == o.vlan
      && ip == o.ip
  ;
  }

};

typedef enum {
  DHCP_WAITING = 0,
  DHCP_FAILED,
  DHCP_SUCCESS
} DHCPSTATUS;

struct l3info {
  DHCPSTATUS status = DHCP_WAITING;
  String ip;
  String gw;
  String netMask;
  String ntp;
  String ns;
  String time;
  String domain;
  String netbios;
  String lease;
  String dhcp;
  String tftp;
  String pxefile;
  String searchdomain;
  
  bool operator==(const l3info& o) const {
    return
      status == o.status
      && ip == o.ip
      && gw == o.gw
      && netMask == o.netMask
      && ntp == o.ntp
      && ns == o.ns
      && time == o.time
      && domain == o.domain
      && netbios == o.netbios
      && lease == o.lease
      && dhcp == o.dhcp
      && tftp == o.tftp
      && pxefile == o.pxefile
      && searchdomain == o.searchdomain
    ;
  }
    
  String netMaskCIDR() {
    unsigned short netmask_cidr;
    int ipbytes[4];
    netmask_cidr=0;
    sscanf(netMask.c_str(), "%d.%d.%d.%d", &ipbytes[0], &ipbytes[1], &ipbytes[2], &ipbytes[3]);
      for (int i=0; i<4; i++)
      {
          switch(ipbytes[i])
          {
              case 0x80: netmask_cidr+=1; break;
              case 0xC0: netmask_cidr+=2; break;
              case 0xE0: netmask_cidr+=3; break;
              case 0xF0: netmask_cidr+=4; break;
              case 0xF8: netmask_cidr+=5; break;
              case 0xFC: netmask_cidr+=6; break;
              case 0xFE: netmask_cidr+=7; break;
              case 0xFF: netmask_cidr+=8; break;
              default: return String(netmask_cidr); break;
          }
      }

      return String(netmask_cidr);
  }
};

struct ldbinfo {
  bool d = true;
  l1info l1;
  l2info l2_cdp;
  l2info l2_lldp;
  l3info l3;

  bool operator==(const ldbinfo& o) const {
      return
        l1 == o.l1
        && l2_cdp == o.l2_cdp
        && l2_lldp == o.l2_lldp
        && l3 == o.l3
      ;
  }

  bool isDirty() {
    return d;
  }
  void dirty() {
    d = true;
  }
  void clean() {
    d = false;
  }
};

#endif // LDBINFO_H
