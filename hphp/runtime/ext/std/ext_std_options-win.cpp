/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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
#include "hphp/runtime/ext/std/ext_std_options.h"

#include <folly/Format.h>
#include <folly/portability/Windows.h>

#include "hphp/runtime/base/strings.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

#ifdef _WIN32
const char* php_get_edition_name(DWORD majVer, DWORD minVer) {
  DWORD productType;
  if (!GetProductInfo(6, 0, 0, 0, &productType)) {
    return nullptr;
  }

  switch (productType) {
  case PRODUCT_ULTIMATE:
    return "Ultimate Edition";
  case PRODUCT_HOME_BASIC:
    return "Home Basic Edition";
  case PRODUCT_HOME_PREMIUM:
    return "Home Premium Edition";
  case PRODUCT_ENTERPRISE:
    return "Enterprise Edition";
  case PRODUCT_HOME_BASIC_N:
    return "Home Basic N Edition";
  case PRODUCT_BUSINESS:
    if ((majVer > 6) || (majVer == 6 && minVer > 0)) {
      return "Professional Edition";
    } else {
      return "Business Edition";
    }
  case PRODUCT_STANDARD_SERVER:
    return "Standard Edition";
  case PRODUCT_DATACENTER_SERVER:
    return "Datacenter Edition";
  case PRODUCT_SMALLBUSINESS_SERVER:
    return "Small Business Server";
  case PRODUCT_ENTERPRISE_SERVER:
    return "Enterprise Edition";
  case PRODUCT_STARTER:
    if ((majVer > 6) || (majVer == 6 && minVer > 0)) {
      return "Starter N Edition";
    } else {
      return "Starter Edition";
    }
  case PRODUCT_DATACENTER_SERVER_CORE:
    return "Datacenter Edition (core installation)";
  case PRODUCT_STANDARD_SERVER_CORE:
    return "Standard Edition (core installation)";
  case PRODUCT_ENTERPRISE_SERVER_CORE:
    return "Enterprise Edition (core installation)";
  case PRODUCT_ENTERPRISE_SERVER_IA64:
    return "Enterprise Edition for Itanium-based Systems";
  case PRODUCT_BUSINESS_N:
    if ((majVer > 6) || (majVer == 6 && minVer > 0)) {
      return "Professional N Edition";
    } else {
      return "Business N Edition";
    }
  case PRODUCT_WEB_SERVER:
    return "Web Server Edition";
  case PRODUCT_CLUSTER_SERVER:
    return "HPC Edition";
  case PRODUCT_HOME_SERVER:
    return "Storage Server Essentials Edition";
  case PRODUCT_STORAGE_EXPRESS_SERVER:
    return "Storage Server Express Edition";
  case PRODUCT_STORAGE_STANDARD_SERVER:
    return "Storage Server Standard Edition";
  case PRODUCT_STORAGE_WORKGROUP_SERVER:
    return "Storage Server Workgroup Edition";
  case PRODUCT_STORAGE_ENTERPRISE_SERVER:
    return "Storage Server Enterprise Edition";
  case PRODUCT_SERVER_FOR_SMALLBUSINESS:
    return "Essential Server Solutions Edition";
  case PRODUCT_SMALLBUSINESS_SERVER_PREMIUM:
    return "Small Business Server Premium Edition";
  case PRODUCT_HOME_PREMIUM_N:
    return "Home Premium N Edition";
  case PRODUCT_ENTERPRISE_N:
    return "Enterprise N Edition";
  case PRODUCT_ULTIMATE_N:
    return "Ultimate N Edition";
  case PRODUCT_WEB_SERVER_CORE:
    return "Web Server Edition (core installation)";
  case PRODUCT_MEDIUMBUSINESS_SERVER_MANAGEMENT:
    return "Essential Business Server Management Server Edition";
  case PRODUCT_MEDIUMBUSINESS_SERVER_SECURITY:
    return "Essential Business Server Management Security Edition";
  case PRODUCT_MEDIUMBUSINESS_SERVER_MESSAGING:
    return "Essential Business Server Management Messaging Edition";
  case PRODUCT_SERVER_FOUNDATION:
    return "Foundation Edition";
  case PRODUCT_HOME_PREMIUM_SERVER:
    return "Home Server 2011 Edition";
  case PRODUCT_SERVER_FOR_SMALLBUSINESS_V:
    return "Essential Server Solutions Edition (without Hyper-V)";
  case PRODUCT_STANDARD_SERVER_V:
    return "Standard Edition (without Hyper-V)";
  case PRODUCT_DATACENTER_SERVER_V:
    return "Datacenter Edition (without Hyper-V)";
  case PRODUCT_ENTERPRISE_SERVER_V:
    return "Enterprise Edition (without Hyper-V)";
  case PRODUCT_DATACENTER_SERVER_CORE_V:
    return "Datacenter Edition (core installation, without Hyper-V)";
  case PRODUCT_STANDARD_SERVER_CORE_V:
    return "Standard Edition (core installation, without Hyper-V)";
  case PRODUCT_ENTERPRISE_SERVER_CORE_V:
    return "Enterprise Edition (core installation, without Hyper-V)";
  case PRODUCT_HYPERV:
    return "Hyper-V Server";
  case PRODUCT_STORAGE_EXPRESS_SERVER_CORE:
    return "Storage Server Express Edition (core installation)";
  case PRODUCT_STORAGE_STANDARD_SERVER_CORE:
    return "Storage Server Standard Edition (core installation)";
  case PRODUCT_STORAGE_WORKGROUP_SERVER_CORE:
    return "Storage Server Workgroup Edition (core installation)";
  case PRODUCT_STORAGE_ENTERPRISE_SERVER_CORE:
    return "Storage Server Enterprise Edition (core installation)";
  case PRODUCT_STARTER_N:
    return "Starter N Edition";
  case PRODUCT_PROFESSIONAL:
    return "Professional Edition";
  case PRODUCT_PROFESSIONAL_N:
    return "Professional N Edition";
  case PRODUCT_SB_SOLUTION_SERVER:
    return "Small Business Server 2011 Essentials Edition";
  case PRODUCT_SERVER_FOR_SB_SOLUTIONS:
    return "Server For SB Solutions Edition";
  case PRODUCT_STANDARD_SERVER_SOLUTIONS:
    return "Solutions Premium Edition";
  case PRODUCT_STANDARD_SERVER_SOLUTIONS_CORE:
    return "Solutions Premium Edition (core installation)";
  case PRODUCT_SB_SOLUTION_SERVER_EM:
    return "Server For SB Solutions EM Edition";
  case PRODUCT_SERVER_FOR_SB_SOLUTIONS_EM:
    return "Server For SB Solutions EM Edition";
  case PRODUCT_SOLUTION_EMBEDDEDSERVER:
    return "MultiPoint Server Edition";
  case PRODUCT_ESSENTIALBUSINESS_SERVER_MGMT:
    return "Essential Server Solution Management Edition";
  case PRODUCT_ESSENTIALBUSINESS_SERVER_ADDL:
    return "Essential Server Solution Additional Edition";
  case PRODUCT_ESSENTIALBUSINESS_SERVER_MGMTSVC:
    return "Essential Server Solution Management SVC Edition";
  case PRODUCT_ESSENTIALBUSINESS_SERVER_ADDLSVC:
    return "Essential Server Solution Additional SVC Edition";
  case PRODUCT_SMALLBUSINESS_SERVER_PREMIUM_CORE:
    return "Small Business Server Premium Edition (core installation)";
  case PRODUCT_CLUSTER_SERVER_V:
    return "Hyper Core V Edition";
  case PRODUCT_STARTER_E:
    return "Hyper Core V Edition";
  case PRODUCT_ENTERPRISE_EVALUATION:
    return "Enterprise Edition (evaluation installation)";
  case PRODUCT_MULTIPOINT_STANDARD_SERVER:
    return "MultiPoint Server Standard Edition (full installation)";
  case PRODUCT_MULTIPOINT_PREMIUM_SERVER:
    return "MultiPoint Server Premium Edition (full installation)";
  case PRODUCT_STANDARD_EVALUATION_SERVER:
    return "Standard Edition (evaluation installation)";
  case PRODUCT_DATACENTER_EVALUATION_SERVER:
    return "Datacenter Edition (evaluation installation)";
  case PRODUCT_ENTERPRISE_N_EVALUATION:
    return "Enterprise N Edition (evaluation installation)";
  case PRODUCT_STORAGE_WORKGROUP_EVALUATION_SERVER:
    return "Storage Server Workgroup Edition (evaluation installation)";
  case PRODUCT_STORAGE_STANDARD_EVALUATION_SERVER:
    return "Storage Server Standard Edition (evaluation installation)";
  case PRODUCT_CORE_N:
    return "Windows 8 N Edition";
  case PRODUCT_CORE_COUNTRYSPECIFIC:
    return "Windows 8 China Edition";
  case PRODUCT_CORE_SINGLELANGUAGE:
    return "Windows 8 Single Language Edition";
  case PRODUCT_CORE:
    return "Windows 8 Edition";
  case PRODUCT_PROFESSIONAL_WMC:
    return "Professional with Media Center Edition";
  }

  return nullptr;
}

Optional<String> php_get_windows_name() {
  const char* majorName = nullptr;
  const char* edName = nullptr;

  OSVERSIONINFOEX verInf;
  verInf.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
  GetVersionEx((OSVERSIONINFO*)&verInf);
  if (verInf.dwPlatformId != VER_PLATFORM_WIN32_NT) {
    return std::nullopt;
  }

  // These are accessed a lot, so use short names to keep things readable.
  DWORD majVer = verInf.dwMajorVersion;
  DWORD minVer = verInf.dwMinorVersion;
  bool isWorkstation = verInf.wProductType == VER_NT_WORKSTATION;

  if (majVer >= 10) {
    if (majVer == 10 && minVer == 0) {
      majorName = isWorkstation ? "Windows 10" : "Windows Server 2016";
    }
  } else if (majVer >= 6) {
    if (majVer == 6) {
      switch (minVer) {
      case 0:
        majorName = isWorkstation ? "Windows Vista" : "Windows Server 2008";
        break;
      case 1:
        majorName = isWorkstation ? "Windows 7" : "Windows Server 2008 R2";
        break;
      case 2: {
        // could be Windows 8/Windows Server 2012, could be Windows 8.1/Windows
        // Server 2012 R2
        // XXX and one more X - the above comment is true if no manifest is
        // used for two cases:
        // - if the PHP build doesn't use the correct manifest
        // - if PHP DLL loaded under some binary that doesn't use the
        //   correct manifest
        //
        // So keep the handling here as is for now, even if we know 6.2 is
        // win8 and nothing else, and think about an improvement.
        OSVERSIONINFOEX osvi81;
        DWORDLONG dwlConditionMask = 0;
        int op = VER_GREATER_EQUAL;

        ZeroMemory(&osvi81, sizeof(OSVERSIONINFOEX));
        osvi81.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
        osvi81.dwMajorVersion = 6;
        osvi81.dwMinorVersion = 3;
        osvi81.wServicePackMajor = 0;

        VER_SET_CONDITION(dwlConditionMask, VER_MAJORVERSION, op);
        VER_SET_CONDITION(dwlConditionMask, VER_MINORVERSION, op);
        VER_SET_CONDITION(dwlConditionMask, VER_SERVICEPACKMAJOR, op);

        if (VerifyVersionInfo(&osvi81,
          VER_MAJORVERSION | VER_MINORVERSION | VER_SERVICEPACKMAJOR,
          dwlConditionMask)) {
          minVer = 3; /* Windows 8.1/Windows Server 2012 R2 */
          majorName = isWorkstation ? "Windows 8.1" : "Windows Server 2012 R2";
        } else {
          majorName = isWorkstation ? "Windows 8" : "Windows Server 2012";
        }
        break;
      }
      case 3:
        majorName = isWorkstation ? "Windows 8.1" : "Windows Server 2012 R2";
        break;
      default:
        majorName = "Unknown Windows version";
        break;
      }

      edName = php_get_edition_name(majVer, minVer);
    }
  }

  return folly::sformat("{}{}{}{}{}",
    majorName,
    edName ? " " : "",
    edName,
    verInf.szCSDVersion[0] != '\0' ? " " : "",
    verInf.szCSDVersion);
}

const StaticString
  s_Windows_NT("Windows NT"),
  s_IA64("IA64"),
  s_IA32("IA32"),
  s_AMD64("AMD64"),
  s_Unknown("Unknown");

String php_get_windows_cpu() {
  SYSTEM_INFO SysInfo;
  GetSystemInfo(&SysInfo);
  switch (SysInfo.wProcessorArchitecture) {
    case PROCESSOR_ARCHITECTURE_INTEL:
      return folly::sformat("i{}", SysInfo.dwProcessorType);
    case PROCESSOR_ARCHITECTURE_MIPS:
      return folly::sformat("MIPS R{}000", SysInfo.wProcessorLevel);
    case PROCESSOR_ARCHITECTURE_ALPHA:
      return folly::sformat("Alpha {}", SysInfo.wProcessorLevel);
    case PROCESSOR_ARCHITECTURE_PPC:
      return folly::sformat("PPC 6{:02}", SysInfo.wProcessorLevel);
    case PROCESSOR_ARCHITECTURE_IA64:
      return s_IA64;
    case PROCESSOR_ARCHITECTURE_IA32_ON_WIN64:
      return s_IA32;
    case PROCESSOR_ARCHITECTURE_AMD64:
      return s_AMD64;
    case PROCESSOR_ARCHITECTURE_UNKNOWN:
    default:
      return s_Unknown;
  }
}
#endif

///////////////////////////////////////////////////////////////////////////////
}
