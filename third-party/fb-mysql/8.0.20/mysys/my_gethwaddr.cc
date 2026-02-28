/* Copyright (c) 2004, 2019, Oracle and/or its affiliates. All rights reserved.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License, version 2.0,
   as published by the Free Software Foundation.

   This program is also distributed with certain software (including
   but not limited to OpenSSL) that is licensed under separate terms,
   as designated in a particular file or component or in included license
   documentation.  The authors of MySQL hereby grant you an additional
   permission to link the program and your derivative works with the
   separately licensed software that they have included with MySQL.

   Without limiting anything contained in the foregoing, this file,
   which is part of C Driver for MySQL (Connector/C), is also subject to the
   Universal FOSS Exception, version 1.0, a copy of which can be found at
   http://oss.oracle.com/licenses/universal-foss-exception.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License, version 2.0, for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA */

/**
  @file mysys/my_gethwaddr.cc
  Get hardware address for an interface.
  If there are many available, any non-zero one can be used.
*/

#include "my_config.h"

#include <errno.h>
#include <string.h>
#ifdef SYS_SOCKET_H
#include <sys/socket.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "my_inttypes.h"
#include "my_sys.h"  // IWYU pragma: keep
#if defined(_WIN32)
#include "mysys/mysys_priv.h"
#endif
#include "mysql/service_mysql_alloc.h"

#ifndef MAIN

#ifdef __FreeBSD__

#include <net/ethernet.h>
#include <net/if.h>
#include <net/if_dl.h>
#include <net/route.h>
#include <sys/sysctl.h>

bool my_gethwaddr(uchar *to) {
  size_t len;
  char *buf, *next, *end;
  struct if_msghdr *ifm;
  struct sockaddr_dl *sdl;
  int res = 1, mib[6] = {CTL_NET, AF_ROUTE, 0, AF_LINK, NET_RT_IFLIST, 0};
  char zero_array[ETHER_ADDR_LEN] = {0};

  if (sysctl(mib, 6, NULL, &len, NULL, 0) == -1) goto err;
  if (!(buf = static_cast<char *>(alloca(len)))) goto err;
  if (sysctl(mib, 6, buf, &len, NULL, 0) < 0) goto err;

  end = buf + len;

  for (next = buf; res && next < end; next += ifm->ifm_msglen) {
    ifm = (struct if_msghdr *)next;
    if (ifm->ifm_type == RTM_IFINFO) {
      sdl = (struct sockaddr_dl *)(ifm + 1);
      memcpy(to, LLADDR(sdl), ETHER_ADDR_LEN);
      res = memcmp(to, zero_array, ETHER_ADDR_LEN) ? 0 : 1;
    }
  }

err:
  return res;
}

#elif defined(__linux__)

#include <net/ethernet.h>
#include <net/if.h>
#include <sys/ioctl.h>

#define MAX_IFS 64

bool my_gethwaddr(uchar *to) {
  int fd = -1;
  int res = 1;
  struct ifreq ifr;
  struct ifreq ifs[MAX_IFS];
  struct ifreq *ifri = nullptr;
  struct ifreq *ifend = nullptr;

  char zero_array[ETHER_ADDR_LEN] = {0};
  struct ifconf ifc;

  fd = socket(AF_INET, SOCK_DGRAM, 0);
  if (fd < 0) return true;

  /* Retrieve interfaces */
  ifc.ifc_len = sizeof(ifs);
  ifc.ifc_req = ifs;
  if (ioctl(fd, SIOCGIFCONF, &ifc) < 0) {
    close(fd);
    return true;
  }

  /* Initialize out parameter */
  memcpy(to, zero_array, ETHER_ADDR_LEN);

  /* Calculate first address after array */
  ifend = ifs + (ifc.ifc_len / sizeof(struct ifreq));

  /* Loop over all interfaces */
  for (ifri = ifc.ifc_req; ifri < ifend; ifri++) {
    if (ifri->ifr_addr.sa_family == AF_INET) {
      /* Reset struct, copy interface name */
      memset(&ifr, 0, sizeof(ifr));
      memcpy(ifr.ifr_name, ifri->ifr_name, sizeof(ifr.ifr_name));

      /* Get HW address, break if not 0 */
      if (ioctl(fd, SIOCGIFHWADDR, &ifr) >= 0) {
        memcpy(to, &ifr.ifr_hwaddr.sa_data, ETHER_ADDR_LEN);
        if (memcmp(to, zero_array, ETHER_ADDR_LEN)) {
          res = 0;
          break;
        }
      }
    }
  }
  close(fd);
  return res;
}

#elif defined(_WIN32)

/*
  Workaround for BUG#32082 (Definition of VOID in my_global.h conflicts with
  windows headers)
*/
#ifdef VOID
#undef VOID
#define VOID void
#endif

#include <iphlpapi.h>

/*
  The following typedef is for dynamically loading iphlpapi.dll /
  GetAdaptersAddresses. Dynamic loading is used because
  GetAdaptersAddresses is not available on Windows 2000 which MySQL
  still supports. Static linking would cause an unresolved export.
*/
typedef DWORD(WINAPI *pfnGetAdaptersAddresses)(
    IN ULONG Family, IN DWORD Flags, IN PVOID Reserved,
    OUT PIP_ADAPTER_ADDRESSES pAdapterAddresses, IN OUT PULONG pOutBufLen);

/*
  my_gethwaddr - Windows version

  @brief Retrieve MAC address from network hardware

  @param[out]  to MAC address exactly six bytes

  @return Operation status
    @retval 0       OK
    @retval <>0     FAILED
*/
bool my_gethwaddr(uchar *to) {
  PIP_ADAPTER_ADDRESSES pAdapterAddresses;
  PIP_ADAPTER_ADDRESSES pCurrAddresses;
  IP_ADAPTER_ADDRESSES adapterAddresses;
  ULONG address_len;
  bool return_val = 1;
  static pfnGetAdaptersAddresses fnGetAdaptersAddresses =
      (pfnGetAdaptersAddresses)-1;

  if (fnGetAdaptersAddresses == (pfnGetAdaptersAddresses)-1) {
    /* Get the function from the DLL */
    fnGetAdaptersAddresses = (pfnGetAdaptersAddresses)GetProcAddress(
        LoadLibrary("iphlpapi.dll"), "GetAdaptersAddresses");
  }
  if (!fnGetAdaptersAddresses) return 1; /* failed to get function */
  address_len = sizeof(IP_ADAPTER_ADDRESSES);

  /* Get the required size for the address data. */
  if (fnGetAdaptersAddresses(AF_UNSPEC, 0, 0, &adapterAddresses,
                             &address_len) == ERROR_BUFFER_OVERFLOW) {
    pAdapterAddresses = static_cast<PIP_ADAPTER_ADDRESSES>(
        my_malloc(key_memory_win_IP_ADAPTER_ADDRESSES, address_len, 0));
    if (!pAdapterAddresses) return 1; /* error, alloc failed */
  } else
    pAdapterAddresses = &adapterAddresses; /* one is enough don't alloc */

  /* Get the hardware info. */
  if (fnGetAdaptersAddresses(AF_UNSPEC, 0, 0, pAdapterAddresses,
                             &address_len) == NO_ERROR) {
    pCurrAddresses = pAdapterAddresses;

    while (pCurrAddresses) {
      /* Look for ethernet cards. */
      if (pCurrAddresses->IfType == IF_TYPE_ETHERNET_CSMACD) {
        /* check for a good address */
        if (pCurrAddresses->PhysicalAddressLength < 6)
          continue; /* bad address */

        /* save 6 bytes of the address in the 'to' parameter */
        memcpy(to, pCurrAddresses->PhysicalAddress, 6);

        /* Network card found, we're done. */
        return_val = 0;
        break;
      }
      pCurrAddresses = pCurrAddresses->Next;
    }
  }

  /* Clean up memory allocation. */
  if (pAdapterAddresses != &adapterAddresses) my_free(pAdapterAddresses);

  return return_val;
}

#else /* __FreeBSD__ || __linux__ || _WIN32 */
/* just fail */
bool my_gethwaddr(uchar *to MY_ATTRIBUTE((unused))) { return 1; }
#endif

#else /* MAIN */
int main(int argc MY_ATTRIBUTE((unused)), char **argv) {
  uchar mac[6];
  uint i;
  MY_INIT(argv[0]);
  if (my_gethwaddr(mac)) {
    printf("my_gethwaddr failed with errno %d\n", errno);
    exit(1);
  }
  for (i = 0; i < sizeof(mac); i++) {
    if (i) printf(":");
    printf("%02x", mac[i]);
  }
  printf("\n");
  return 0;
}
#endif
