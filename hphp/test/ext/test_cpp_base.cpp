/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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
#include "hphp/runtime/server/virtual-host.h"
#include "hphp/runtime/server/satellite-server.h"
#include "hphp/system/systemlib.h"

///////////////////////////////////////////////////////////////////////////////

TestCppBase::TestCppBase() {
}

///////////////////////////////////////////////////////////////////////////////

bool TestCppBase::RunTests(const std::string &which) {
  bool ret = true;
  RUN_TEST(TestIpBlockMap);
  RUN_TEST(TestIpBlockMapIni);
  RUN_TEST(TestVirtualHost);
  RUN_TEST(TestVirtualHostIni);
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
    "IpBlockMap {\n"
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
    "}\n"
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

bool TestCppBase::TestIpBlockMapIni() {
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

  std::string inistr =
    "hhvm.ip_block_map[0][location] = /test\n"
    "hhvm.ip_block_map[0][allow_first] = true\n"
    "hhvm.ip_block_map[0][ip][allow][0] = 127.0.0.1\n"
    "hhvm.ip_block_map[0][ip][deny][0] = 8.32.0.0/24\n"
    "hhvm.ip_block_map[0][ip][deny][1] = "
    "aaaa:bbbb:cccc:dddd:eeee:ffff:1111::/80\n";

  IniSetting::Map ini = IniSetting::Map::object;
  Hdf empty;

  Config::ParseIniString(inistr, ini);

  IpBlockMap ibm(ini, empty);

  VERIFY(!ibm.isBlocking("test/blah.php", "127.0.0.1"));
  VERIFY(ibm.isBlocking("test/blah.php", "8.32.0.104"));
  VERIFY(ibm.isBlocking("test/blah.php",
                        "aaaa:bbbb:cccc:dddd:eeee:9999:8888:7777"));
  // allow first
  VERIFY(!ibm.isBlocking("test/blah.php",
                         "aaaa:bbbb:cccc:dddd:eee3:4444:3333:2222"));

  return Count(true);
}

bool TestCppBase::TestVirtualHost() {
  IniSetting::Map ini = IniSetting::Map::object;
  Hdf hdf;
  hdf.fromString(
    "  Server {\n"
    "    AllowedDirectories.* = /var/www\n"
    "    AllowedDirectories.* = /usr/bin\n"
    "  }\n"
    "  VirtualHost {\n"
    "    flibtest {\n"
    "      Prefix = flibtest.\n"
    "      PathTranslation = flib/_bin\n"
    "      ServerName = flibtest.facebook.com\n"
    "    }\n"
    "    upload {\n"
    "      Prefix = upload.\n"
    "      ServerVariables {\n"
    "        TFBENV = 8{\n"
    "      }\n"
    "      overwrite {\n"
    "        Server {\n"
    "          AllowedDirectories.* = /var/www\n"
    "          AllowedDirectories.* = /mnt\n"
    "          AllowedDirectories.* = /tmp\n"
    "          AllowedDirectories.* = /var/tmp/tao\n"
    "        }\n"
    "        MaxPostSize = 100MB\n"
    "        UploadMaxFileSize = 100MB\n"
    "        RequestTimeoutSeconds = 120\n"
    "      }\n"
    "      PathTranslation = html\n"
    "    }\n"
    "    default {\n"
    "      LogFilters {\n"
    "        * {\n"
    "          url = /method/searchme\n"
    "          params {\n"
    "            * = q\n"
    "            * = s\n"
    "            * = atoken\n"
    "            * = otoken\n"
    "          }\n"
    "        value = REMOVED\n"
    "        }\n"
    "      }\n"
    "      RewriteRules {\n"
    "        common {\n"
    "          pattern = /html/common/\n"
    "          to = http://3v4l.org\n"
    "          qsa = true\n"
    "          redirect = 301\n"
    "        }\n"
    "      }\n"
    "      PathTranslation = htm\n"
    "    }\n"
    "  }\n"
  );

  // reset RuntimeOption::AllowedDirectories to empty because if the INI
  // version of this test is run at the same time, we don't want to append
  // the same directories to it. We want to start fresh.
  RuntimeOption::AllowedDirectories.clear();
  std::vector<VirtualHost> hosts;
  RuntimeOption::AllowedDirectories =
    Config::GetStrVector(ini, hdf, "Server.AllowedDirectories");
  auto cb = [&] (const IniSetting::Map &ini_cb, const Hdf &hdf_cb,
                 const std::string &host) {
    if (VirtualHost::IsDefault(ini_cb, hdf_cb, host)) {
      VirtualHost::GetDefault().init(ini_cb, hdf_cb, host);
      VirtualHost::GetDefault().
        addAllowedDirectories(RuntimeOption::AllowedDirectories);
    } else {
      auto vh = VirtualHost(ini_cb, hdf_cb, host);
      // These will be added
      // "    AllowedDirectories.* = /var/www\n"
      // "    AllowedDirectories.* = /usr/bin\n"
      vh.addAllowedDirectories(RuntimeOption::AllowedDirectories);
      hosts.push_back(vh);
    }
  };
  Config::Iterate(cb, ini, hdf, "VirtualHost");

  for (auto& host : hosts) {
    VirtualHost::SetCurrent(&host);
    auto name = host.getName();
    if (name == "flibtest") {
      VERIFY(host.getPathTranslation() == "flib/_bin/"); // the / is added
      VERIFY(host.getDocumentRoot() ==
             RuntimeOption::SourceRoot + "flib/_bin");
      VERIFY(host.getServerVars().size() == 0);
      VERIFY(VirtualHost::GetAllowedDirectories().size() == 2);
      VERIFY(host.valid() == true);
      VERIFY(host.hasLogFilter() == false);
    } else if (name == "upload") {
      VERIFY(host.getPathTranslation() == "html/"); // the / is added
      VERIFY(host.getDocumentRoot() ==
             RuntimeOption::SourceRoot + "html");
      // SortALlowedDirectories might add something and remove
      // duplicates. In this case, /var/releases/continuous_www_scripts4
      // was added and the duplicate /var/www was removed.s
      VERIFY(VirtualHost::GetAllowedDirectories().size() == 6);
      VERIFY(host.valid() == true);
      VERIFY(host.hasLogFilter() == false);
      VERIFY(VirtualHost::GetMaxPostSize() == 104857600);
    }
  }

  VERIFY(VirtualHost::GetDefault().getPathTranslation() == "htm/");
  VERIFY(VirtualHost::GetDefault().hasLogFilter() == true);
  String rw("/html/common/");
  String h("default");
  bool q = false;
  int rd = 0;
  VirtualHost::GetDefault().rewriteURL(h, rw, q, rd);
  VERIFY(rw.toCppString() == "http://3v4l.org");
  VERIFY(rd == 301);
  return Count(true);
}

bool TestCppBase::TestVirtualHostIni() {
  std::string inistr =
    "hhvm.server.allowed_directories[] = /var/www\n"
    "hhvm.server.allowed_directories[] = /usr/bin\n"
    "hhvm.virtual_host[flibtest][prefix] = flibtest.\n"
    "hhvm.virtual_host[flibtest][path_translation] = flib/_bin\n"
    "hhvm.virtual_host[flibtest][server_name] = flibtest.facebook.com\n"\
    "hhvm.virtual_host[flibtest][log_filters][0][url] = function/searchme\n"
    "hhvm.virtual_host[flibtest][log_filters][0][params][0] = v\n"
    "hhvm.virtual_host[flibtest][log_filters][0][params][1] = t\n"
    "hhvm.virtual_host[flibtest][log_filters][0][params][2] = btoken\n"
    "hhvm.virtual_host[flibtest][log_filters][0][params][3] = ptoken\n"
    "hhvm.virtual_host[flibtest][log_filters][0][value] = INSERTED\n"
    "hhvm.virtual_host[flibtest][log_filters][1][url] = property/searchme\n"
    "hhvm.virtual_host[flibtest][log_filters][1][pattern] = #thePattern#\n"
    "hhvm.virtual_host[flibtest][log_filters][1][value] = BETWEEN\n"
    "hhvm.virtual_host[upload][prefix] = upload.\n"
    "hhvm.virtual_host[upload][server_variables][TFBENV] = 8\n"
    "hhvm.virtual_host[upload][overwrite][server][allowed_directories][0] = "
    "/var/www\n"
    "hhvm.virtual_host[upload][overwrite][server][allowed_directories][1] = "
    "/mnt\n"
    "hhvm.virtual_host[upload][overwrite][server][allowed_directories][2] = "
    "/tmp\n"
    "hhvm.virtual_host[upload][overwrite][server][allowed_directories][3] = "
    "/var/tmp/tao\n"
    "hhvm.virtual_host[upload][overwrite][server][max_post_size] = 100MB\n"
    "hhvm.virtual_host[upload][overwrite][server][upload]"
    "[upload_max_file_size] = 100MB\n"
    "hhvm.virtual_host[upload][overwrite][server]"
    "[request_timeout_seconds] = 120\n"
    "hhvm.virtual_host[upload][path_translation] = html\n"
    "hhvm.virtual_host[default][path_translation] = htm\n"
    "hhvm.virtual_host[default][log_filters][0][url] = method/searchme\n"
    "hhvm.virtual_host[default][log_filters][0][params][0] = q\n"
    "hhvm.virtual_host[default][log_filters][0][params][1] = s\n"
    "hhvm.virtual_host[default][log_filters][0][params][2] = atoken\n"
    "hhvm.virtual_host[default][log_filters][0][params][3] = otoken\n"
    "hhvm.virtual_host[default][log_filters][0][value] = REMOVED\n"
    "hhvm.virtual_host[default][rewrite_rules][common][pattern] = "
    "/html/common/\n"
    "hhvm.virtual_host[default][rewrite_rules][common][to] = "
    "http://3v4l.org\n"
    "hhvm.virtual_host[default][rewrite_rules][common][qsa] = true\n"
    "hhvm.virtual_host[default][rewrite_rules][common][redirect] = 301\n";

  IniSetting::Map ini = IniSetting::Map::object;
  Hdf empty;
  std::vector<VirtualHost> hosts;

  // reset RuntimeOption::AllowedDirectories to empty because if the Hdf
  // version of this test is run at the same time, we don't want to append
  // the same directories to it. We want to start fresh.
  RuntimeOption::AllowedDirectories.clear();

  Config::ParseIniString(inistr, ini);
  RuntimeOption::AllowedDirectories =
    Config::GetStrVector(ini, empty, "Server.AllowedDirectories");
  auto cb = [&] (const IniSetting::Map &ini_cb,
                 const Hdf &hdf_cb,
                 const std::string &host){
    if (VirtualHost::IsDefault(ini_cb, hdf_cb, host)) {
      VirtualHost::GetDefault().init(ini_cb, hdf_cb, host);
      VirtualHost::GetDefault().
        addAllowedDirectories(RuntimeOption::AllowedDirectories);
    } else {
      auto vh = VirtualHost(ini_cb, hdf_cb, host);
      // These will be added
      // "hhvm.server.allowed_directories[] = /var/www\n"
      // "hhvm.server.allowed_directories[] = /usr/bin\n"
      vh.addAllowedDirectories(RuntimeOption::AllowedDirectories);
      hosts.push_back(vh);
    }
  };
  Config::Iterate(cb, ini, empty, "VirtualHost");

  for (auto& host : hosts) {
    VirtualHost::SetCurrent(&host);
    auto name = host.getName();
    if (name == "flibtest") {
      VERIFY(host.getPathTranslation() == "flib/_bin/"); // the / is added
      VERIFY(host.getDocumentRoot() ==
             RuntimeOption::SourceRoot + "flib/_bin");
      VERIFY(host.getServerVars().size() == 0);
      VERIFY(VirtualHost::GetAllowedDirectories().size() == 2);
      VERIFY(host.valid() == true);
      VERIFY(host.hasLogFilter() == true);
      VERIFY(host.filterUrl("http://function/searchme/bar?ptoken=baz") ==
                            "http://function/searchme/bar?ptoken=INSERTED");
      VERIFY(host.filterUrl(
        "http://function/searchme/thePattern?ptoken=baz") ==
        "http://function/searchme/thePattern?ptoken=INSERTED"
      );
      VERIFY(host.filterUrl("http://property/searchme/foothePattern") ==
                            "http://property/searchme/foo=BETWEEN");
      VERIFY(host.filterUrl("foo") == "foo");
    } else if (name == "upload") {
      VERIFY(host.getPathTranslation() == "html/"); // the / is added
      VERIFY(host.getDocumentRoot() ==
             RuntimeOption::SourceRoot + "html");
      // SortALlowedDirectories might add something and remove
      // duplicates. In this case, /var/releases/continuous_www_scripts4
      // was added and the duplicate /var/www was removed.s
      VERIFY(VirtualHost::GetAllowedDirectories().size() == 6);
      VERIFY(host.valid() == true);
      VERIFY(host.hasLogFilter() == false);
      VERIFY(VirtualHost::GetMaxPostSize() == 104857600);
    }
  }

  VERIFY(VirtualHost::GetDefault().getPathTranslation() == "htm/");
  VERIFY(VirtualHost::GetDefault().hasLogFilter() == true);
  String rw("/html/common/");
  String h("default");
  bool q = false;
  int rd = 0;
  VirtualHost::GetDefault().rewriteURL(h, rw, q, rd);
  VERIFY(rw.toCppString() == "http://3v4l.org");
  VERIFY(rd == 301);

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

  Config::Bind(RuntimeOption::AllowedDirectories, ini,
               hdf, "Server.AllowedDirectories");
  VERIFY(RuntimeOption::AllowedDirectories.size() == 2);
  std::vector<std::string> ad =
    Config::GetStrVector(ini, hdf, "Server.AllowedDirectories",
                      RuntimeOption::AllowedDirectories);
  VERIFY(RuntimeOption::AllowedDirectories.size() == 2);
  VERIFY(ad.size() == 2);
  Config::Bind(RuntimeOption::ServerHighPriorityEndPoints, ini,
               hdf, "Server.HighPriorityEndPoints");
  VERIFY(RuntimeOption::ServerHighPriorityEndPoints.size() == 3);
  return Count(true);
}

bool TestCppBase::TestCollectionIni() {
  std::string inistr =
    "hhvm.server.allowed_directories[] = /var/www\n"
    "hhvm.server.allowed_directories[] = /usr/bin\n"
    "hhvm.server.high_priority_end_points[] = /end\n"
    "hhvm.server.high_priority_end_points[] = /point\n"
    "hhvm.server.high_priority_end_points[] = /power\n"
    "hhvm.server.high_priority_end_points[] = /word\n";

  IniSetting::Map ini = IniSetting::Map::object;
  Hdf empty;

  // Ensure we have no residuals left over from the HDF run.
  RuntimeOption::AllowedDirectories.clear();
  RuntimeOption::ServerHighPriorityEndPoints.clear();

  Config::ParseIniString(inistr, ini);
  Config::Bind(RuntimeOption::AllowedDirectories, ini, empty,
               "Server.AllowedDirectories"); // Test converting name
  VERIFY(RuntimeOption::AllowedDirectories.size() == 2);
  std::vector<std::string> ad =
    Config::GetStrVector(ini, empty, "Server.AllowedDirectories",
                      RuntimeOption::AllowedDirectories, false);
  // This should still be 2. In other words, Get shouldn't append
  // values.
  VERIFY(RuntimeOption::AllowedDirectories.size() == 2);
  VERIFY(ad.size() == 2);
  Config::Bind(RuntimeOption::ServerHighPriorityEndPoints, ini, empty,
               "Server.HighPriorityEndPoints",
                RuntimeOption::ServerHighPriorityEndPoints, false);
  VERIFY(RuntimeOption::ServerHighPriorityEndPoints.size() == 4);
  return Count(true);
}
