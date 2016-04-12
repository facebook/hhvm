/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
   | Copyright (c) 1997-2010 The PHP Group                                |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/

#ifndef incl_HPHP_EXT_NETWORK_INTERNAL_H_
#define incl_HPHP_EXT_NETWORK_INTERNAL_H_

// HOST_NAME_MAX is recommended by POSIX, but not required.
// FreeBSD and OSX (as of 10.9) are known to not define it.
// 255 is generally the safe value to assume and upstream
// PHP does this as well.
#ifndef HOST_NAME_MAX
#define HOST_NAME_MAX 255
#endif

#define MAXPACKET  8192 /* max packet size used internally by BIND */
#define DNS_T_A 1
#define DNS_T_NS 2
#define DNS_T_CNAME 5
#define DNS_T_SOA 6
#define DNS_T_PTR 12
#define DNS_T_HINFO 13
#define DNS_T_MINFO 14
#define DNS_T_MX 15
#define DNS_T_TXT 16
#define DNS_T_AAAA 28
#define DNS_T_SRV 33
#define DNS_T_NAPTR 35
#define DNS_T_A6 38
#define DNS_T_ANY 255

#define PHP_DNS_NUM_TYPES   12  // Number of DNS Types Supported by PHP
#define PHP_DNS_A      0x00000001
#define PHP_DNS_NS     0x00000002
#define PHP_DNS_CNAME  0x00000010
#define PHP_DNS_SOA    0x00000020
#define PHP_DNS_PTR    0x00000800
#define PHP_DNS_HINFO  0x00001000
#define PHP_DNS_MX     0x00004000
#define PHP_DNS_TXT    0x00008000
#define PHP_DNS_A6     0x01000000
#define PHP_DNS_SRV    0x02000000
#define PHP_DNS_NAPTR  0x04000000
#define PHP_DNS_AAAA   0x08000000
#define PHP_DNS_ANY    0x10000000
#define PHP_DNS_ALL    (PHP_DNS_A|PHP_DNS_NS|PHP_DNS_CNAME|PHP_DNS_SOA| \
                        PHP_DNS_PTR|PHP_DNS_HINFO|PHP_DNS_MX|PHP_DNS_TXT| \
                        PHP_DNS_A6|PHP_DNS_SRV|PHP_DNS_NAPTR|PHP_DNS_AAAA)

#endif // incl_HPHP_EXT_NETWORK_INTERNAL_H_
