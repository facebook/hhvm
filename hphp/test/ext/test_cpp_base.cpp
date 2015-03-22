/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/test/ext/test_cpp_base.h"
#include "hphp/runtime/ext/extension.h"
#include "hphp/util/logger.h"
#include "hphp/runtime/base/memory-manager.h"
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/ext/apc/ext_apc.h"
#include "hphp/runtime/ext/string/ext_string.h"
#include "hphp/runtime/ext/std/ext_std_variable.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/server/ip-block-map.h"
#include "hphp/system/systemlib.h"

///////////////////////////////////////////////////////////////////////////////

TestCppBase::TestCppBase() {
}

///////////////////////////////////////////////////////////////////////////////

bool TestCppBase::RunTests(const std::string &which) {
  bool ret = true;
  RUN_TEST(TestIpBlockMap);
  RUN_TEST(TestCollectionHdf);
  RUN_TEST(TestCollectionIni);
  return ret;
}

///////////////////////////////////////////////////////////////////////////////
// building blocks

/* Pull 32bit Big Endian words from an in6_addr */
static inline long in6addrWord(struct in6_addr addr, char wordNo) {
  return ((addr.s6_addr[(wordNo*4)+0] << 24) |
          (addr.s6_addr[(wordNo*4)+1] << 16) |
          (addr.s6_addr[(wordNo*4)+2] <<  8) |
          (addr.s6_addr[(wordNo*4)+3] <<  0)) & 0xFFFFFFFF;
}

bool TestCppBase::TestIpBlockMap() {
  struct in6_addr addr;
  int bits;

  VERIFY(IpBlockMap::ReadIPv6Address("204.15.21.0/22", &addr, bits));
  VS(bits, 118);
  VS(in6addrWord(addr, 0), 0x00000000L);
  VS(in6addrWord(addr, 1), 0x00000000L);
  VS(in6addrWord(addr, 2), 0x0000FFFFL);
  VS(in6addrWord(addr, 3), 0xCC0F1500L);

  VERIFY(IpBlockMap::ReadIPv6Address("127.0.0.1", &addr, bits));
  VS(bits, 128);
  VS(in6addrWord(addr, 0), 0x00000000L);
  VS(in6addrWord(addr, 1), 0x00000000L);
  VS(in6addrWord(addr, 2), 0x0000FFFFL);
  VS(in6addrWord(addr, 3), 0x7F000001L);

  VERIFY(IpBlockMap::ReadIPv6Address(
    "1111:2222:3333:4444:5555:6666:789a:bcde", &addr, bits));
  VS(bits, 128);
  VS(in6addrWord(addr, 0), 0x11112222L);
  VS(in6addrWord(addr, 1), 0x33334444L);
  VS(in6addrWord(addr, 2), 0x55556666L);
  VS(in6addrWord(addr, 3), 0x789abcdeL);

  VERIFY(IpBlockMap::ReadIPv6Address(
    "1111:2222:3333:4444:5555:6666:789a:bcde/68", &addr, bits));
  VS(bits, 68);
  VS(in6addrWord(addr, 0), 0x11112222L);
  VS(in6addrWord(addr, 1), 0x33334444L);
  VS(in6addrWord(addr, 2), 0x55556666L);
  VS(in6addrWord(addr, 3), 0x789abcdeL);

  IpBlockMap::BinaryPrefixTrie root(true);
  unsigned char value[16];

  // Default value with no additional nodes
  memset(value, 0, 16);
  VERIFY(root.isAllowed(value, 1));
  value[0] = 0x80;
  VERIFY(root.isAllowed(value));

  // Inheritance of parent allow value through multiple levels of new nodes
  IpBlockMap::BinaryPrefixTrie::InsertNewPrefix(&root, value, 1, false);
  value[0] = 0xf0;
  IpBlockMap::BinaryPrefixTrie::InsertNewPrefix(&root, value, 4, true);
  VERIFY(root.isAllowed(value));
  value[0] = 0xe0;
  VERIFY(!root.isAllowed(value));
  value[0] = 0xc0;
  VERIFY(!root.isAllowed(value));
  value[0] = 0x80;
  VERIFY(!root.isAllowed(value));
  value[0] = 0;
  VERIFY(root.isAllowed(value));

  // > 1 byte in address
  value[2] = 0xff;
  IpBlockMap::BinaryPrefixTrie::InsertNewPrefix(&root, value, 24, false);
  VERIFY(!root.isAllowed(value));
  value[3] = 0xff;
  VERIFY(!root.isAllowed(value));
  value[2] = 0xfe;
  VERIFY(root.isAllowed(value));

  // Exact address match
  value[2]  = 0xff;
  value[15] = 1;
  IpBlockMap::BinaryPrefixTrie::InsertNewPrefix(&root, value, 128, true);
  VERIFY(root.isAllowed(value));

  IniSetting::Map ini = IniSetting::Map::object;
  Hdf hdf;
  hdf.fromString(
    "  0 {\n"
    "    Location = /test\n"
    "    AllowFirst = true\n"
    "    Ip {\n"
    "      Allow {\n"
    "       * = 127.0.0.1\n"
    "     }\n"
    "     Deny {\n"
    "       * = 8.32.0.0/24\n"
    "       * = aaaa:bbbb:cccc:dddd:eeee:ffff:1111::/80\n"
    "     }\n"
    "    }\n"
    "  }\n"
  );

  IpBlockMap ibm(ini, hdf);
  VERIFY(!ibm.isBlocking("test/blah.php", "127.0.0.1"));
  VERIFY(ibm.isBlocking("test/blah.php", "8.32.0.104"));
  VERIFY(ibm.isBlocking("test/blah.php",
                        "aaaa:bbbb:cccc:dddd:eeee:9999:8888:7777"));
  VERIFY(!ibm.isBlocking("test/blah.php",
                         "aaaa:bbbb:cccc:dddd:eee3:4444:3333:2222"));

  return Count(true);
}

bool TestCppBase::TestCollectionHdf() {
  IniSetting::Map ini = IniSetting::Map::object;
  Hdf hdf;
  hdf.fromString(
    "  Server {\n"
    "    AllowedDirectories.* = /var/www\n"
    "    AllowedDirectories.* = /usr/bin\n"
    "    HighPriorityEndPoints.* = /end\n"
    "    HighPriorityEndPoints.* = /point\n"
    "    HighPriorityEndPoints.* = /power\n"
    "  }\n"
  );
  RuntimeOption::AllowedDirectories.clear();

  Hdf server = hdf["Server"];

  Config::Bind(RuntimeOption::AllowedDirectories, ini,
               server["AllowedDirectories"]);
  VERIFY(RuntimeOption::AllowedDirectories.size() == 2);
  std::vector<std::string> ad =
    Config::GetVector(ini, server["AllowedDirectories"],
                      RuntimeOption::AllowedDirectories);
  VERIFY(RuntimeOption::AllowedDirectories.size() == 2);
  VERIFY(ad.size() == 2);
  Config::Bind(RuntimeOption::ServerHighPriorityEndPoints, ini,
               server["HighPriorityEndPoints"]);
  VERIFY(RuntimeOption::ServerHighPriorityEndPoints.size() == 3);
  return Count(true);
}

bool TestCppBase::TestCollectionIni() {
  std::string inistr =
    "hhvm.server.allowed_directories[] = /var/www\n"
    "hhvm.server.allowed_directories[] = /usr/bin\n"
    "hhvm.server.high_priority_endpoints[] = /end\n"
    "hhvm.server.high_priority_endpoints[] = /point\n"
    "hhvm.server.high_priority_endpoints[] = /power\n";

  IniSetting::Map ini = IniSetting::Map::object;

  RuntimeOption::AllowedDirectories.clear();
  Config::ParseIniString(inistr, ini);
  Config::Bind(RuntimeOption::AllowedDirectories, ini,
               "hhvm.server.allowed_directories");
  VERIFY(RuntimeOption::AllowedDirectories.size() == 2);
  std::vector<std::string> ad =
    Config::GetVector(ini, "hhvm.server.allowed_directories",
                      RuntimeOption::AllowedDirectories);
  // This should still be 2. In other words, Get shouldn't append
  // values.
  VERIFY(RuntimeOption::AllowedDirectories.size() == 2);
  VERIFY(ad.size() == 2);
  Config::Bind(RuntimeOption::ServerHighPriorityEndPoints, ini,
               "hhvm.server.high_priority_endpoints");
  VERIFY(RuntimeOption::ServerHighPriorityEndPoints.size() == 3);
  return Count(true);
}
