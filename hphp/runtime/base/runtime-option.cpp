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

#include "hphp/runtime/base/runtime-option.h"

#include <cstdint>
#include <limits>
#include <stdexcept>
#include <map>
#include <memory>
#include <set>
#include <vector>

#include <sys/time.h>
#include <sys/resource.h>

#include "folly/String.h"

#include "hphp/util/hdf.h"
#include "hphp/util/text-util.h"
#include "hphp/util/network.h"
#include "hphp/util/logger.h"
#include "hphp/util/stack-trace.h"
#include "hphp/util/process.h"
#include "hphp/util/file-cache.h"
#include "hphp/util/log-file-flusher.h"
#include "hphp/runtime/base/file-util.h"

#include "hphp/parser/scanner.h"

#include "hphp/runtime/server/satellite-server.h"
#include "hphp/runtime/server/virtual-host.h"
#include "hphp/runtime/server/files-match.h"
#include "hphp/runtime/server/access-log.h"

#include "hphp/runtime/base/type-conversions.h"
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/base/shared-store-base.h"
#include "hphp/runtime/base/extended-logger.h"
#include "hphp/runtime/base/simple-counter.h"
#include "hphp/runtime/base/memory-manager.h"
#include "hphp/runtime/base/hardware-counter.h"
#include "hphp/runtime/base/preg.h"
#include "hphp/runtime/base/crash-reporter.h"
#include "hphp/runtime/base/static-string-table.h"
#include "hphp/runtime/base/config.h"
#include "hphp/runtime/base/ini-setting.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

const char *RuntimeOption::ExecutionMode = "";
std::string RuntimeOption::BuildId;
std::string RuntimeOption::InstanceId;
std::string RuntimeOption::PidFile = "www.pid";

std::string RuntimeOption::LogFile;
std::string RuntimeOption::LogFileSymLink;
int RuntimeOption::LogHeaderMangle = 0;
bool RuntimeOption::AlwaysEscapeLog = false;
bool RuntimeOption::AlwaysLogUnhandledExceptions = true;
bool RuntimeOption::NoSilencer = false;
bool RuntimeOption::CallUserHandlerOnFatals = true;
bool RuntimeOption::ThrowExceptionOnBadMethodCall = true;
int RuntimeOption::RuntimeErrorReportingLevel =
  static_cast<int>(ErrorConstants::ErrorModes::HPHP_ALL);

std::string RuntimeOption::ServerUser;

int RuntimeOption::MaxLoopCount = 0;
int RuntimeOption::MaxSerializedStringSize = 64 * 1024 * 1024; // 64MB
size_t RuntimeOption::ArrUnserializeCheckSize = 10000;
bool RuntimeOption::NoInfiniteRecursionDetection = false;
bool RuntimeOption::WarnTooManyArguments = false;
bool RuntimeOption::EnableHipHopErrors = true;
bool RuntimeOption::AssertActive = false;
bool RuntimeOption::AssertWarning = false;
int64_t RuntimeOption::NoticeFrequency = 1;
int64_t RuntimeOption::WarningFrequency = 1;
int RuntimeOption::RaiseDebuggingFrequency = 1;
int64_t RuntimeOption::SerializationSizeLimit = StringData::MaxSize;
int64_t RuntimeOption::StringOffsetLimit = 10 * 1024 * 1024; // 10MB

std::string RuntimeOption::AccessLogDefaultFormat = "%h %l %u %t \"%r\" %>s %b";
std::vector<AccessLogFileData> RuntimeOption::AccessLogs;

std::string RuntimeOption::AdminLogFormat = "%h %t %s %U";
std::string RuntimeOption::AdminLogFile;
std::string RuntimeOption::AdminLogSymLink;


std::string RuntimeOption::Tier;
std::string RuntimeOption::Host;
std::string RuntimeOption::DefaultServerNameSuffix;
std::string RuntimeOption::ServerType = "libevent";
std::string RuntimeOption::ServerIP;
std::string RuntimeOption::ServerFileSocket;
std::string RuntimeOption::ServerPrimaryIP;
int RuntimeOption::ServerPort = 80;
int RuntimeOption::ServerPortFd = -1;
int RuntimeOption::ServerBacklog = 128;
int RuntimeOption::ServerConnectionLimit = 0;
int RuntimeOption::ServerThreadCount = 50;
int RuntimeOption::ProdServerPort = 80;
int RuntimeOption::QueuedJobsReleaseRate = 3;
bool RuntimeOption::ServerThreadRoundRobin = false;
int RuntimeOption::ServerWarmupThrottleRequestCount = 0;
int RuntimeOption::ServerThreadDropCacheTimeoutSeconds = 0;
int RuntimeOption::ServerThreadJobLIFOSwitchThreshold = INT_MAX;
int RuntimeOption::ServerThreadJobMaxQueuingMilliSeconds = -1;
bool RuntimeOption::ServerThreadDropStack = false;
bool RuntimeOption::ServerHttpSafeMode = false;
bool RuntimeOption::ServerStatCache = false;
bool RuntimeOption::ServerFixPathInfo = false;
std::vector<std::string> RuntimeOption::ServerWarmupRequests;
boost::container::flat_set<std::string>
RuntimeOption::ServerHighPriorityEndPoints;
bool RuntimeOption::ServerExitOnBindFail;
int RuntimeOption::PageletServerThreadCount = 0;
bool RuntimeOption::PageletServerThreadRoundRobin = false;
int RuntimeOption::PageletServerThreadDropCacheTimeoutSeconds = 0;
int RuntimeOption::PageletServerQueueLimit = 0;
bool RuntimeOption::PageletServerThreadDropStack = false;
int RuntimeOption::RequestTimeoutSeconds = 0;
int RuntimeOption::PspTimeoutSeconds = 0;
int64_t RuntimeOption::ServerMemoryHeadRoom = 0;
int64_t RuntimeOption::RequestMemoryMaxBytes =
  std::numeric_limits<int64_t>::max();
int64_t RuntimeOption::ImageMemoryMaxBytes = 0;
int RuntimeOption::ResponseQueueCount = 0;
int RuntimeOption::ServerGracefulShutdownWait = 0;
bool RuntimeOption::ServerHarshShutdown = true;
bool RuntimeOption::ServerEvilShutdown = true;
bool RuntimeOption::ServerKillOnSIGTERM = false;
int RuntimeOption::ServerDanglingWait = 0;
int RuntimeOption::ServerShutdownListenWait = 0;
int RuntimeOption::ServerShutdownListenNoWork = -1;
std::vector<std::string> RuntimeOption::ServerNextProtocols;
int RuntimeOption::GzipCompressionLevel = 3;
std::string RuntimeOption::ForceCompressionURL;
std::string RuntimeOption::ForceCompressionCookie;
std::string RuntimeOption::ForceCompressionParam;
bool RuntimeOption::EnableMagicQuotesGpc = false;
bool RuntimeOption::EnableKeepAlive = true;
bool RuntimeOption::ExposeHPHP = true;
bool RuntimeOption::ExposeXFBServer = false;
bool RuntimeOption::ExposeXFBDebug = false;
std::string RuntimeOption::XFBDebugSSLKey;
int RuntimeOption::ConnectionTimeoutSeconds = -1;
bool RuntimeOption::EnableOutputBuffering = false;
std::string RuntimeOption::OutputHandler;
bool RuntimeOption::ImplicitFlush = false;
bool RuntimeOption::EnableEarlyFlush = true;
bool RuntimeOption::ForceChunkedEncoding = false;
int64_t RuntimeOption::MaxPostSize = 100;
bool RuntimeOption::AlwaysPopulateRawPostData = true;
int64_t RuntimeOption::UploadMaxFileSize = 100;
std::string RuntimeOption::UploadTmpDir = "/tmp";
bool RuntimeOption::EnableFileUploads = true;
bool RuntimeOption::EnableUploadProgress = false;
int RuntimeOption::Rfc1867Freq = 256 * 1024;
std::string RuntimeOption::Rfc1867Prefix = "vupload_";
std::string RuntimeOption::Rfc1867Name = "video_ptoken";
bool RuntimeOption::LibEventSyncSend = true;
bool RuntimeOption::ExpiresActive = true;
int RuntimeOption::ExpiresDefault = 2592000;
std::string RuntimeOption::DefaultCharsetName = "utf-8";
bool RuntimeOption::ForceServerNameToHeader = false;
bool RuntimeOption::EnableCufAsync = false;
bool RuntimeOption::PathDebug = false;

int RuntimeOption::RequestBodyReadLimit = -1;

bool RuntimeOption::EnableSSL = false;
int RuntimeOption::SSLPort = 443;
int RuntimeOption::SSLPortFd = -1;
std::string RuntimeOption::SSLCertificateFile;
std::string RuntimeOption::SSLCertificateKeyFile;
std::string RuntimeOption::SSLCertificateDir;
bool RuntimeOption::TLSDisableTLS1_2 = false;
std::string RuntimeOption::TLSClientCipherSpec;

std::vector<std::shared_ptr<VirtualHost>> RuntimeOption::VirtualHosts;
std::shared_ptr<IpBlockMap> RuntimeOption::IpBlocks;
std::vector<std::shared_ptr<SatelliteServerInfo>>
  RuntimeOption::SatelliteServerInfos;

int RuntimeOption::XboxServerThreadCount = 10;
int RuntimeOption::XboxServerMaxQueueLength = INT_MAX;
int RuntimeOption::XboxServerPort = 0;
int RuntimeOption::XboxDefaultLocalTimeoutMilliSeconds = 500;
int RuntimeOption::XboxDefaultRemoteTimeoutSeconds = 5;
int RuntimeOption::XboxServerInfoMaxRequest = 500;
int RuntimeOption::XboxServerInfoDuration = 120;
std::string RuntimeOption::XboxServerInfoWarmupDoc;
std::string RuntimeOption::XboxServerInfoReqInitFunc;
std::string RuntimeOption::XboxServerInfoReqInitDoc;
bool RuntimeOption::XboxServerInfoAlwaysReset = false;
bool RuntimeOption::XboxServerLogInfo = false;
std::string RuntimeOption::XboxProcessMessageFunc = "xbox_process_message";
std::string RuntimeOption::XboxPassword;
std::set<std::string> RuntimeOption::XboxPasswords;

std::string RuntimeOption::SourceRoot = Process::GetCurrentDirectory() + '/';
std::vector<std::string> RuntimeOption::IncludeSearchPaths;
std::string RuntimeOption::FileCache;
std::string RuntimeOption::DefaultDocument;
std::string RuntimeOption::ErrorDocument404;
bool RuntimeOption::ForbiddenAs404 = false;
std::string RuntimeOption::ErrorDocument500;
std::string RuntimeOption::FatalErrorMessage;
std::string RuntimeOption::FontPath;
bool RuntimeOption::EnableStaticContentFromDisk = true;
bool RuntimeOption::EnableOnDemandUncompress = true;
bool RuntimeOption::EnableStaticContentMMap = true;

bool RuntimeOption::Utf8izeReplace = true;

std::string RuntimeOption::StartupDocument;
std::string RuntimeOption::WarmupDocument;
std::string RuntimeOption::RequestInitFunction;
std::string RuntimeOption::RequestInitDocument;
std::string RuntimeOption::AutoPrependFile;
std::string RuntimeOption::AutoAppendFile;

bool RuntimeOption::SafeFileAccess = false;
std::vector<std::string> RuntimeOption::AllowedDirectories;
std::set<std::string> RuntimeOption::AllowedFiles;
hphp_string_imap<std::string> RuntimeOption::StaticFileExtensions;
hphp_string_imap<std::string> RuntimeOption::PhpFileExtensions;
std::set<std::string> RuntimeOption::ForbiddenFileExtensions;
std::set<std::string> RuntimeOption::StaticFileGenerators;
std::vector<std::shared_ptr<FilesMatch>> RuntimeOption::FilesMatches;

bool RuntimeOption::WhitelistExec = false;
bool RuntimeOption::WhitelistExecWarningOnly = false;
std::vector<std::string> RuntimeOption::AllowedExecCmds;

bool RuntimeOption::UnserializationWhitelistCheck = false;
bool RuntimeOption::UnserializationWhitelistCheckWarningOnly = true;

std::string RuntimeOption::TakeoverFilename;
int RuntimeOption::AdminServerPort = 0;
int RuntimeOption::AdminThreadCount = 1;
std::string RuntimeOption::AdminPassword;
std::set<std::string> RuntimeOption::AdminPasswords;

std::string RuntimeOption::ProxyOrigin;
int RuntimeOption::ProxyRetry = 3;
bool RuntimeOption::UseServeURLs;
std::set<std::string> RuntimeOption::ServeURLs;
bool RuntimeOption::UseProxyURLs;
int RuntimeOption::ProxyPercentage = 0;
std::set<std::string> RuntimeOption::ProxyURLs;
std::vector<std::string> RuntimeOption::ProxyPatterns;
bool RuntimeOption::AlwaysUseRelativePath = false;

int RuntimeOption::HttpDefaultTimeout = 30;
int RuntimeOption::HttpSlowQueryThreshold = 5000; // ms

bool RuntimeOption::TranslateLeakStackTrace = false;
bool RuntimeOption::NativeStackTrace = false;
bool RuntimeOption::FullBacktrace = false;
bool RuntimeOption::ServerErrorMessage = false;
bool RuntimeOption::TranslateSource = false;
bool RuntimeOption::RecordInput = false;
bool RuntimeOption::ClearInputOnSuccess = true;
std::string RuntimeOption::ProfilerOutputDir = "/tmp";
std::string RuntimeOption::CoreDumpEmail;
bool RuntimeOption::CoreDumpReport = true;
std::string RuntimeOption::CoreDumpReportDirectory
#if defined(HPHP_OSS)
  ("/tmp");
#else
  ("/var/tmp/cores");
#endif
bool RuntimeOption::LocalMemcache = false;
bool RuntimeOption::MemcacheReadOnly = false;

bool RuntimeOption::EnableStats = false;
bool RuntimeOption::EnableAPCStats = false;
bool RuntimeOption::EnableWebStats = false;
bool RuntimeOption::EnableMemoryStats = false;
bool RuntimeOption::EnableMemcacheStats = false;
bool RuntimeOption::EnableMemcacheKeyStats = false;
bool RuntimeOption::EnableSQLStats = false;
bool RuntimeOption::EnableSQLTableStats = false;
bool RuntimeOption::EnableNetworkIOStatus = false;
std::string RuntimeOption::StatsXSL;
std::string RuntimeOption::StatsXSLProxy;
int RuntimeOption::StatsSlotDuration = 10 * 60; // 10 minutes
int RuntimeOption::StatsMaxSlot = 12 * 6; // 12 hours

int64_t RuntimeOption::MaxRSS = 0;
int64_t RuntimeOption::MaxRSSPollingCycle = 0;
int64_t RuntimeOption::DropCacheCycle = 0;
int64_t RuntimeOption::MaxSQLRowCount = 0;
int64_t RuntimeOption::MaxMemcacheKeyCount = 0;
int64_t RuntimeOption::SocketDefaultTimeout = 5;
bool RuntimeOption::LockCodeMemory = false;
int RuntimeOption::MaxArrayChain = INT_MAX;
bool RuntimeOption::WarnOnCollectionToArray = false;
bool RuntimeOption::UseDirectCopy = false;

bool RuntimeOption::EnableDnsCache = false;
int RuntimeOption::DnsCacheTTL = 10 * 60; // 10 minutes

std::map<std::string, std::string> RuntimeOption::ServerVariables;
std::map<std::string, std::string> RuntimeOption::EnvVariables;

std::string RuntimeOption::LightProcessFilePrefix = "./lightprocess";
int RuntimeOption::LightProcessCount = 0;

bool RuntimeOption::EnableHipHopSyntax = false;
bool RuntimeOption::EnableHipHopExperimentalSyntax = false;
bool RuntimeOption::EnableShortTags = true;
bool RuntimeOption::EnableAspTags = false;
bool RuntimeOption::EnableXHP = false;
bool RuntimeOption::EnableObjDestructCall = true;
bool RuntimeOption::EnableEmitSwitch = true;
bool RuntimeOption::EnableEmitterStats = true;
bool RuntimeOption::CheckSymLink = true;
int RuntimeOption::MaxUserFunctionId = (2 * 65536);
bool RuntimeOption::EnableArgsInBacktraces = true;
bool RuntimeOption::EnableZendCompat = false;
bool RuntimeOption::EnableZendSorting = false;
bool RuntimeOption::TimeoutsUseWallTime = true;
bool RuntimeOption::CheckFlushOnUserClose = true;
bool RuntimeOption::EvalAuthoritativeMode = false;
bool RuntimeOption::IntsOverflowToInts = false;

#ifdef HHVM_DYNAMIC_EXTENSION_DIR
std::string RuntimeOption::ExtensionDir = HHVM_DYNAMIC_EXTENSION_DIR;
#else
std::string RuntimeOption::ExtensionDir = "";
#endif

std::vector<std::string> RuntimeOption::Extensions;
std::vector<std::string> RuntimeOption::DynamicExtensions;
std::string RuntimeOption::DynamicExtensionPath = ".";

std::vector<void(*)(const IniSettingMap&, const Hdf&)>*
  RuntimeOption::OptionHooks = nullptr;

void RuntimeOption::AddOptionHook(
  void(*optionHook)(const IniSettingMap&, const Hdf&)) {
  // assuming no concurrent call to this function
  if (RuntimeOption::OptionHooks == nullptr) {
    RuntimeOption::OptionHooks =
      new std::vector<void(*)(const IniSettingMap&, const Hdf&)>();
  }
  RuntimeOption::OptionHooks->push_back(optionHook);
}

HackStrictOption
  RuntimeOption::StrictArrayFillKeys = HackStrictOption::OFF,
  RuntimeOption::DisallowDynamicVarEnvFuncs = HackStrictOption::OFF;
bool RuntimeOption::LookForTypechecker = true;

int RuntimeOption::GetScannerType() {
  int type = 0;
  if (EnableShortTags) type |= Scanner::AllowShortTags;
  if (EnableAspTags) type |= Scanner::AllowAspTags;
  if (EnableXHP) type |= Scanner::AllowXHPSyntax;
  if (EnableHipHopSyntax) type |= Scanner::AllowHipHopSyntax;
  return type;
}

bool RuntimeOption::GetServerCustomBoolSetting(const std::string &settingName,
                                               bool &val) {
  auto it = RuntimeOption::CustomSettings.find(settingName);
  if (it == RuntimeOption::CustomSettings.end()) {
    // The value isn't present in the CustomSettings section
    return false;
  }

  val = Hdf::convertRawConfigToBool(it->second.data());
  return true;
}

static inline std::string regionSelectorDefault() {
  return "tracelet";
}

static inline bool pgoDefault() {
  // TODO(3496304)
  return !RuntimeOption::EvalSimulateARM;
}

static inline bool loopsDefault() {
#ifdef HHVM_JIT_LOOPS_BY_DEFAULT
  return true;
#else
  return false;
#endif
}

static inline bool controlFlowDefault() {
#if defined(HHVM_JIT_LOOPS_BY_DEFAULT) || defined(HHVM_CONTROL_FLOW)
  return true;
#else
  return false;
#endif
}

static inline bool evalJitDefault() {
#if defined(__APPLE__) || defined(__CYGWIN__)
  return false;
#else
  return true;
#endif
}

static inline bool simulateARMDefault() {
#ifdef HHVM_SIMULATE_ARM_BY_DEFAULT
  return true;
#else
  return false;
#endif
}

static inline bool jitPseudomainDefault() {
  // TODO(#4238120)
  return !RuntimeOption::EvalSimulateARM;
}

static inline bool hugePagesSoundNice() {
  return RuntimeOption::ServerExecutionMode();
}

const uint64_t kEvalVMStackElmsDefault =
#ifdef VALGRIND
 0x800
#else
 0x4000
#endif
 ;
const uint32_t kEvalVMInitialGlobalTableSizeDefault = 512;
static const int kDefaultProfileInterpRequests = debug ? 1 : 11;
static const int kDefaultJitPGOThreshold = debug ? 2 : 10;
static const uint32_t kDefaultProfileRequests = debug ? 1 << 31 : 500;
static const size_t kJitGlobalDataDef = RuntimeOption::EvalJitASize >> 2;

static const bool kJitTimerDefault =
#ifdef ENABLE_JIT_TIMER_DEFAULT
  true
#else
  false
#endif
;

inline size_t maxUsageDef() {
  return RuntimeOption::EvalJitASize;
}
using std::string;
#define F(type, name, def) \
  type RuntimeOption::Eval ## name = type(def);
EVALFLAGS();
#undef F
std::set<string, stdltistr> RuntimeOption::DynamicInvokeFunctions;
bool RuntimeOption::RecordCodeCoverage = false;
std::string RuntimeOption::CodeCoverageOutputFile;

std::string RuntimeOption::RepoLocalMode;
std::string RuntimeOption::RepoLocalPath;
std::string RuntimeOption::RepoCentralPath;
std::string RuntimeOption::RepoEvalMode;
std::string RuntimeOption::RepoJournal = "delete";
bool RuntimeOption::RepoCommit = true;
bool RuntimeOption::RepoDebugInfo = true;
// Missing: RuntimeOption::RepoAuthoritative's physical location is
// perf-sensitive.
bool RuntimeOption::RepoPreload;

bool RuntimeOption::SandboxMode = false;
std::string RuntimeOption::SandboxPattern;
std::string RuntimeOption::SandboxHome;
std::string RuntimeOption::SandboxFallback;
std::string RuntimeOption::SandboxConfFile;
std::map<std::string, std::string> RuntimeOption::SandboxServerVariables;
bool RuntimeOption::SandboxFromCommonRoot = false;
std::string RuntimeOption::SandboxDirectoriesRoot;
std::string RuntimeOption::SandboxLogsRoot;

bool RuntimeOption::EnableDebugger = false;
bool RuntimeOption::EnableDebuggerColor = true;
bool RuntimeOption::EnableDebuggerPrompt = true;
bool RuntimeOption::EnableDebuggerServer = false;
bool RuntimeOption::EnableDebuggerUsageLog = false;
bool RuntimeOption::DebuggerDisableIPv6 = false;
int RuntimeOption::DebuggerServerPort = 8089;
int RuntimeOption::DebuggerDefaultRpcPort = 8083;
std::string RuntimeOption::DebuggerDefaultRpcAuth;
std::string RuntimeOption::DebuggerRpcHostDomain;
int RuntimeOption::DebuggerDefaultRpcTimeout = 30;
std::string RuntimeOption::DebuggerDefaultSandboxPath;
std::string RuntimeOption::DebuggerStartupDocument;
int RuntimeOption::DebuggerSignalTimeout = 1;

std::string RuntimeOption::SendmailPath = "sendmail -t -i";
std::string RuntimeOption::MailForceExtraParameters;

int64_t RuntimeOption::PregBacktraceLimit = 1000000;
int64_t RuntimeOption::PregRecursionLimit = 100000;
bool RuntimeOption::EnablePregErrorLog = true;

bool RuntimeOption::HHProfServerEnabled = false;
int RuntimeOption::HHProfServerPort = 4327;
int RuntimeOption::HHProfServerThreads = 2;
int RuntimeOption::HHProfServerTimeoutSeconds = 30;
bool RuntimeOption::HHProfServerProfileClientMode = true;
bool RuntimeOption::HHProfServerAllocationProfile = false;
int RuntimeOption::HHProfServerFilterMinAllocPerReq = 2;
int RuntimeOption::HHProfServerFilterMinBytesPerReq = 128;

bool RuntimeOption::SimpleXMLEmptyNamespaceMatchesAll = false;

bool RuntimeOption::AllowDuplicateCookies = true;

bool RuntimeOption::EnableHotProfiler = true;
int RuntimeOption::ProfilerTraceBuffer = 2000000;
double RuntimeOption::ProfilerTraceExpansion = 1.2;
int RuntimeOption::ProfilerMaxTraceBuffer = 0;

#ifdef FACEBOOK
bool RuntimeOption::EnableFb303Server = true;
int RuntimeOption::Fb303ServerPort = 0;
int RuntimeOption::Fb303ServerThreadStackSizeMb = 8;
int RuntimeOption::Fb303ServerWorkerThreads = 1;
int RuntimeOption::Fb303ServerPoolThreads = 1;
#endif

double RuntimeOption::XenonPeriodSeconds = 0.0;
bool RuntimeOption::XenonForceAlwaysOn = false;

std::map<std::string, std::string> RuntimeOption::CustomSettings;

int RuntimeOption::EnableAlternative = 0;

///////////////////////////////////////////////////////////////////////////////

static void setResourceLimit(int resource, const IniSetting::Map& ini,
                             Hdf rlimit, const char *nodeName) {
  if (!Config::GetString(ini, rlimit[nodeName]).empty()) {
    struct rlimit rl;
    getrlimit(resource, &rl);
    rl.rlim_cur = Config::GetInt64(ini, rlimit[nodeName]);
    if (rl.rlim_max < rl.rlim_cur) {
      rl.rlim_max = rl.rlim_cur;
    }
    int ret = setrlimit(resource, &rl);
    if (ret) {
      Logger::Error("Unable to set %s to %" PRId64 ": %s (%d)",
                    nodeName, (int64_t)rl.rlim_cur,
                    folly::errnoStr(errno).c_str(), errno);
    }
  }
}

static void normalizePath(std::string &path) {
  if (!path.empty()) {
    if (path[path.length() - 1] == '/') {
      path = path.substr(0, path.length() - 1);
    }
    if (path[0] != '/') {
      path = std::string("/") + path;
    }
  }
}

static bool matchHdfPattern(const std::string &value, const IniSetting::Map& ini, Hdf hdfPattern) {
  string pattern = Config::GetString(ini, hdfPattern);
  if (!pattern.empty()) {
    Variant ret = preg_match(String(pattern.c_str(), pattern.size(),
                                    CopyString),
                             String(value.c_str(), value.size(),
                                    CopyString));
    if (ret.toInt64() <= 0) {
      return false;
    }
  }
  return true;
}

void RuntimeOption::Load(const IniSetting::Map& ini,
                         Hdf& config,
                         std::vector<std::string> *overwrites /* = nullptr */) {
  if (overwrites) {
    // Do these first, mainly so we can override Tier.*.machine,
    // Tier.*.tier and Tier.*.cpu on the command line. But it can
    // also make sense to override fields within a Tier (
    // eg if you are using the same command line across a lot
    // of different machines)
    for (unsigned int i = 0; i < overwrites->size(); i++) {
      config.fromString(overwrites->at(i).c_str());
    }
  }

  // Machine metrics
  string hostname, tier, cpu;
  {
    Hdf machine = config["Machine"];

    hostname = Config::GetString(ini, machine["name"]);
    if (hostname.empty()) {
      hostname = Process::GetHostName();
    }

    tier = Config::GetString(ini, machine["tier"]);

    cpu = Config::GetString(ini, machine["cpu"]);
    if (cpu.empty()) {
      cpu = Process::GetCPUModel();
    }
  }

  // Tier overwrites
  {
    Hdf tiers = config["Tiers"];
    for (Hdf hdf = tiers.firstChild(); hdf.exists(); hdf = hdf.next()) {
      if (matchHdfPattern(hostname, ini, hdf["machine"]) &&
          matchHdfPattern(tier, ini, hdf["tier"]) &&
          matchHdfPattern(cpu, ini, hdf["cpu"])) {
        Tier = hdf.getName();
        config.copy(hdf["overwrite"]);
        // no break here, so we can continue to match more overwrites
      }
      hdf["overwrite"].setVisited(); // avoid lint complaining
    }
  }

  if (overwrites) {
    // Do the command line overrides again, so we override
    // any tier overwrites
    for (unsigned int i = 0; i < overwrites->size(); i++) {
      config.fromString(overwrites->at(i).c_str());
    }
  }

  Config::Bind(PidFile, ini, config["PidFile"], "www.pid");

  Config::Get(ini, config["DynamicInvokeFunctions"], DynamicInvokeFunctions);

  {
    Hdf logger = config["Log"];
    if (logger["Level"] == "None") {
      Logger::LogLevel = Logger::LogNone;
    } else if (logger["Level"] == "Error") {
      Logger::LogLevel = Logger::LogError;
    } else if (logger["Level"] == "Warning") {
      Logger::LogLevel = Logger::LogWarning;
    } else if (logger["Level"] == "Info") {
      Logger::LogLevel = Logger::LogInfo;
    } else if (logger["Level"] == "Verbose") {
      Logger::LogLevel = Logger::LogVerbose;
    }
    IniSetting::Bind(IniSetting::CORE, IniSetting::PHP_INI_SYSTEM,
                     "hhvm.log.level", IniSetting::SetAndGet<std::string>(
      [](const std::string& value) {
        if (value == "None") {
          Logger::LogLevel = Logger::LogNone;
        } else if (value == "Error") {
          Logger::LogLevel = Logger::LogError;
        } else if (value == "Warning") {
          Logger::LogLevel = Logger::LogWarning;
        } else if (value == "Info") {
          Logger::LogLevel = Logger::LogInfo;
        } else if (value == "Verbose") {
          Logger::LogLevel = Logger::LogVerbose;
        } else {
          return false;
        }
        return true;
      },
      []() {
        switch (Logger::LogLevel) {
          case Logger::LogNone:
            return "None";
          case Logger::LogError:
            return "Error";
          case Logger::LogWarning:
            return "Warning";
          case Logger::LogInfo:
            return "Info";
          case Logger::LogVerbose:
            return "Verbose";
        }
        return "";
      }
    ));

    Config::Bind(Logger::LogHeader, ini, logger["Header"]);
    Config::Bind(Logger::LogNativeStackTrace, ini, logger["NativeStackTrace"],
                 true);
    Config::Bind(Logger::MaxMessagesPerRequest, ini,
                 logger["MaxMessagesPerRequest"], -1);

    Config::Bind(Logger::UseSyslog, ini, logger["UseSyslog"], false);
    Config::Bind(Logger::UseLogFile, ini, logger["UseLogFile"], true);
    Config::Bind(Logger::UseCronolog, ini, logger["UseCronolog"], false);
    Config::Bind(Logger::UseRequestLog, ini, logger["UseRequestLog"], false);
    if (Logger::UseLogFile) {
      Config::Bind(LogFile, ini, logger["File"]);
      if (!RuntimeOption::ServerExecutionMode()) {
        LogFile.clear();
      }
      if (LogFile[0] == '|') Logger::IsPipeOutput = true;
      Config::Bind(LogFileSymLink, ini, logger["SymLink"]);
    }
    Config::Bind(LogFileFlusher::DropCacheChunkSize, ini,
                 logger["DropCacheChunkSize"], 1 << 20);
    Config::Bind(AlwaysEscapeLog, ini, logger["AlwaysEscapeLog"], false);
    Config::Bind(RuntimeOption::LogHeaderMangle, ini, logger["HeaderMangle"],
                 0);

    Config::Bind(AlwaysLogUnhandledExceptions, ini,
                 logger["AlwaysLogUnhandledExceptions"], true);
    Config::Bind(NoSilencer, ini, logger["NoSilencer"]);
    Config::Bind(RuntimeErrorReportingLevel, ini,
                 logger["RuntimeErrorReportingLevel"],
                 static_cast<int>(ErrorConstants::ErrorModes::HPHP_ALL));
    Config::Bind(AccessLogDefaultFormat, ini, logger["AccessLogDefaultFormat"],
                 "%h %l %u %t \"%r\" %>s %b");

    {
      Hdf access = logger["Access"];
      for (Hdf hdf = access.firstChild(); hdf.exists();
           hdf = hdf.next()) {
        string fname = Config::GetString(ini, hdf["File"]);
        if (fname.empty()) {
          continue;
        }
        string symLink = Config::GetString(ini, hdf["SymLink"]);
        AccessLogs.push_back(AccessLogFileData(fname, symLink,
          Config::GetString(ini, hdf["Format"], AccessLogDefaultFormat)));
      }
    }

    Config::Bind(AdminLogFormat, ini, logger["AdminLog.Format"], "%h %t %s %U");
    Config::Bind(AdminLogFile, ini, logger["AdminLog.File"]);
    Config::Bind(AdminLogSymLink, ini, logger["AdminLog.SymLink"]);
  }
  {
    Hdf error = config["ErrorHandling"];

    /* Remove this, once its removed from production configs */
    (void)Config::GetBool(ini, error["NoInfiniteLoopDetection"]);

    Config::Bind(MaxSerializedStringSize, ini,
                 error["MaxSerializedStringSize"], 64 * 1024 * 1024);
    Config::Bind(CallUserHandlerOnFatals, ini,
                 error["CallUserHandlerOnFatals"], true);
    Config::Bind(ThrowExceptionOnBadMethodCall, ini,
                 error["ThrowExceptionOnBadMethodCall"], true);
    Config::Bind(MaxLoopCount, ini, error["MaxLoopCount"], 0);
    Config::Bind(NoInfiniteRecursionDetection, ini,
                 error["NoInfiniteRecursionDetection"]);
    Config::Bind(WarnTooManyArguments, ini, error["WarnTooManyArguments"]);
    Config::Bind(EnableHipHopErrors, ini, error["EnableHipHopErrors"], true);
    Config::Bind(AssertActive, ini, error["AssertActive"]);
    Config::Bind(AssertWarning, ini, error["AssertWarning"]);
    Config::Bind(NoticeFrequency, ini, error["NoticeFrequency"], 1);
    Config::Bind(WarningFrequency, ini, error["WarningFrequency"], 1);
  }
  {
    Hdf rlimit = config["ResourceLimit"];
    if (Config::GetInt64(ini, rlimit["CoreFileSizeOverride"])) {
      setResourceLimit(RLIMIT_CORE, ini, rlimit, "CoreFileSizeOverride");
    } else {
      setResourceLimit(RLIMIT_CORE, ini, rlimit, "CoreFileSize");
    }
    setResourceLimit(RLIMIT_NOFILE, ini, rlimit, "MaxSocket");
    setResourceLimit(RLIMIT_DATA, ini, rlimit, "RSS");
    Config::Bind(MaxRSS, ini, rlimit["MaxRSS"], 0);
    Config::Bind(SocketDefaultTimeout, ini, rlimit["SocketDefaultTimeout"], 5);
    Config::Bind(MaxRSSPollingCycle, ini, rlimit["MaxRSSPollingCycle"], 0);
    Config::Bind(DropCacheCycle, ini, rlimit["DropCacheCycle"], 0);
    Config::Bind(MaxSQLRowCount, ini, rlimit["MaxSQLRowCount"], 0);
    Config::Bind(MaxMemcacheKeyCount, ini, rlimit["MaxMemcacheKeyCount"], 0);
    Config::Bind(SerializationSizeLimit, ini, rlimit["SerializationSizeLimit"],
                 StringData::MaxSize);
    Config::Bind(StringOffsetLimit, ini, rlimit["StringOffsetLimit"],
                 10 * 1024 * 1024);
  }
  {
    Hdf eval = config["Eval"];
    Config::Bind(EnableHipHopSyntax, ini, eval["EnableHipHopSyntax"]);
    Config::Bind(EnableHipHopExperimentalSyntax, ini,
                 eval["EnableHipHopExperimentalSyntax"]);
    Config::Bind(EnableShortTags, ini, eval["EnableShortTags"], true);
    Config::Bind(EnableAspTags, ini, eval["EnableAspTags"]);
    Config::Bind(EnableXHP, ini, eval["EnableXHP"], false);
    Config::Bind(EnableZendCompat, ini, eval["EnableZendCompat"], false);
    Config::Bind(EnableZendSorting, ini, eval["EnableZendSorting"], false);
    Config::Bind(TimeoutsUseWallTime, ini, eval["TimeoutsUseWallTime"], true);
    Config::Bind(CheckFlushOnUserClose, ini, eval["CheckFlushOnUserClose"],
                 true);

    if (EnableHipHopSyntax) {
      // If EnableHipHopSyntax is true, it forces EnableXHP to true
      // regardless of how it was set in the config
      EnableXHP = true;
    }

    Config::Bind(EnableObjDestructCall, ini, eval["EnableObjDestructCall"],
                 true);
    Config::Bind(MaxUserFunctionId, ini, eval["MaxUserFunctionId"], 2 * 65536);
    Config::Bind(CheckSymLink, ini, eval["CheckSymLink"], true);

    Config::Bind(EnableAlternative, ini, eval["EnableAlternative"], 0);
#define F(type, name, defaultVal) \
    Config::Bind(Eval ## name, ini, eval[#name], defaultVal);
    EVALFLAGS()
#undef F
    low_malloc_huge_pages(EvalMaxLowMemHugePages);
    Config::Bind(EnableEmitSwitch, ini, eval["EnableEmitSwitch"], true);
    Config::Bind(EnableEmitterStats, ini, eval["EnableEmitterStats"],
                 EnableEmitterStats);
    Config::Bind(RecordCodeCoverage, ini, eval["RecordCodeCoverage"]);
    if (EvalJit && RecordCodeCoverage) {
      throw std::runtime_error("Code coverage is not supported with "
        "Eval.Jit=true");
    }
    if (RecordCodeCoverage) CheckSymLink = true;
    Config::Bind(CodeCoverageOutputFile, ini, eval["CodeCoverageOutputFile"]);
    {
      Hdf debugger = eval["Debugger"];
      Config::Bind(EnableDebugger, ini, debugger["EnableDebugger"]);
      Config::Bind(EnableDebuggerColor, ini, debugger["EnableDebuggerColor"],
                   true);
      Config::Bind(EnableDebuggerPrompt, ini, debugger["EnableDebuggerPrompt"],
                   true);
      Config::Bind(EnableDebuggerServer, ini, debugger["EnableDebuggerServer"]);
      Config::Bind(EnableDebuggerUsageLog, ini,
                   debugger["EnableDebuggerUsageLog"]);
      Config::Bind(DebuggerServerPort, ini, debugger["Port"], 8089);
      Config::Bind(DebuggerDisableIPv6, ini, debugger["DisableIPv6"], false);
      Config::Bind(DebuggerDefaultSandboxPath, ini,
                   debugger["DefaultSandboxPath"]);
      Config::Bind(DebuggerStartupDocument, ini, debugger["StartupDocument"]);
      Config::Bind(DebuggerSignalTimeout, ini, debugger["SignalTimeout"], 1);

      Config::Bind(DebuggerDefaultRpcPort, ini, debugger["RPC.DefaultPort"],
                   8083);
      Config::Bind(DebuggerDefaultRpcAuth, ini, debugger["RPC.DefaultAuth"]);
      Config::Bind(DebuggerRpcHostDomain, ini, debugger["RPC.HostDomain"]);
      Config::Bind(DebuggerDefaultRpcTimeout, ini,
                   debugger["RPC.DefaultTimeout"], 30);
    }
    {
      Hdf lang = config["Hack"]["Lang"];
      IntsOverflowToInts =
        Config::GetBool(ini, lang["IntsOverflowToInts"], EnableHipHopSyntax);
      Config::Bind(StrictArrayFillKeys, ini, lang["StrictArrayFillKeys"]);
      Config::Bind(DisallowDynamicVarEnvFuncs, ini,
                   lang["DisallowDynamicVarEnvFuncs"]);
      // Defaults to EnableHHSyntax since, if you have that on, you are
      // assumed to know what you're doing.
      Config::Bind(LookForTypechecker, ini, lang["LookForTypechecker"],
                   !EnableHipHopSyntax);
    }
    {
      Hdf repo = config["Repo"];
      {
        Hdf repoLocal = repo["Local"];
        // Repo.Local.Mode.
        Config::Bind(RepoLocalMode, ini, repoLocal["Mode"]);
        if (RepoLocalMode.empty()) {
          const char* HHVM_REPO_LOCAL_MODE = getenv("HHVM_REPO_LOCAL_MODE");
          if (HHVM_REPO_LOCAL_MODE != nullptr) {
            RepoLocalMode = HHVM_REPO_LOCAL_MODE;
          }
          RepoLocalMode = "r-";
        }
        if (RepoLocalMode.compare("rw")
            && RepoLocalMode.compare("r-")
            && RepoLocalMode.compare("--")) {
          Logger::Error("Bad config setting: Repo.Local.Mode=%s",
                        RepoLocalMode.c_str());
          RepoLocalMode = "rw";
        }
        // Repo.Local.Path.
        Config::Bind(RepoLocalPath, ini, repoLocal["Path"]);
        if (RepoLocalPath.empty()) {
          const char* HHVM_REPO_LOCAL_PATH = getenv("HHVM_REPO_LOCAL_PATH");
          if (HHVM_REPO_LOCAL_PATH != nullptr) {
            RepoLocalPath = HHVM_REPO_LOCAL_PATH;
          }
        }
      }
      {
        Hdf repoCentral = repo["Central"];
        // Repo.Central.Path.
        Config::Bind(RepoCentralPath, ini, repoCentral["Path"]);
      }
      {
        Hdf repoEval = repo["Eval"];
        // Repo.Eval.Mode.
        Config::Bind(RepoEvalMode, ini, repoEval["Mode"]);
        if (RepoEvalMode.empty()) {
          RepoEvalMode = "readonly";
        } else if (RepoEvalMode.compare("local")
                   && RepoEvalMode.compare("central")
                   && RepoEvalMode.compare("readonly")) {
          Logger::Error("Bad config setting: Repo.Eval.Mode=%s",
                        RepoEvalMode.c_str());
          RepoEvalMode = "readonly";
        }
      }
      Config::Bind(RepoJournal, ini, repo["Journal"], "delete");
      Config::Bind(RepoCommit, ini, repo["Commit"], true);
      Config::Bind(RepoDebugInfo, ini, repo["DebugInfo"], true);
      Config::Bind(RepoAuthoritative, ini, repo["Authoritative"], false);
      Config::Bind(RepoPreload, ini, repo["Preload"], RepoAuthoritative);
    }

    // NB: after we know the value of RepoAuthoritative.
    Config::Bind(EnableArgsInBacktraces, ini, eval["EnableArgsInBacktraces"],
                 !RepoAuthoritative);
    Config::Bind(EvalAuthoritativeMode, ini, eval["AuthoritativeMode"], false);
    if (RepoAuthoritative) {
      EvalAuthoritativeMode = true;
    }
  }
  {
    Hdf server = config["Server"];
    Config::Bind(Host, ini, server["Host"]);
    Config::Bind(DefaultServerNameSuffix, ini,
                 server["DefaultServerNameSuffix"]);
    Config::Bind(ServerType, ini, server["Type"], ServerType);
    Config::Bind(ServerIP, ini, server["IP"]);
    Config::Bind(ServerFileSocket, ini, server["FileSocket"]);
    ServerPrimaryIP = GetPrimaryIP();
    Config::Bind(ServerPort, ini, server["Port"], 80);
    Config::Bind(ServerBacklog, ini, server["Backlog"], 128);
    Config::Bind(ServerConnectionLimit, ini, server["ConnectionLimit"], 0);
    Config::Bind(ServerThreadCount, ini, server["ThreadCount"],
                 Process::GetCPUCount() * 2);

    Config::Bind(ProdServerPort, ini,
        server["ProdServerPort"], 80);

    Config::Bind(ServerThreadRoundRobin, ini, server["ThreadRoundRobin"]);
    Config::Bind(ServerWarmupThrottleRequestCount, ini,
                 server["WarmupThrottleRequestCount"],
                 ServerWarmupThrottleRequestCount);
    Config::Bind(ServerThreadDropCacheTimeoutSeconds, ini,
                 server["ThreadDropCacheTimeoutSeconds"], 0);
    if (Config::GetBool(ini, server["ThreadJobLIFO"])) {
      ServerThreadJobLIFOSwitchThreshold = 0;
    }
    Config::Bind(ServerThreadJobLIFOSwitchThreshold, ini,
                 server["ThreadJobLIFOSwitchThreshold"],
                 ServerThreadJobLIFOSwitchThreshold);
    Config::Bind(ServerThreadJobMaxQueuingMilliSeconds, ini,
                 server["ThreadJobMaxQueuingMilliSeconds"], -1);
    Config::Bind(ServerThreadDropStack, ini, server["ThreadDropStack"]);
    Config::Bind(ServerHttpSafeMode, ini, server["HttpSafeMode"]);
    Config::Bind(ServerStatCache, ini, server["StatCache"], false);
    Config::Bind(ServerFixPathInfo, ini, server["FixPathInfo"], false);
    Config::Get(ini, server["WarmupRequests"], ServerWarmupRequests);
    Config::Get(ini, server["HighPriorityEndPoints"],
                ServerHighPriorityEndPoints);
    Config::Bind(ServerExitOnBindFail, ini, server["ExitOnBindFail"], false);

    Config::Bind(RequestTimeoutSeconds, ini, server["RequestTimeoutSeconds"],
                 0);
    Config::Bind(PspTimeoutSeconds, ini, server["PspTimeoutSeconds"], 0);
    Config::Bind(ServerMemoryHeadRoom, ini, server["MemoryHeadRoom"], 0);
    Config::Bind(RequestMemoryMaxBytes, ini, server["RequestMemoryMaxBytes"],
                       std::numeric_limits<int64_t>::max());
    Config::Bind(ResponseQueueCount, ini, server["ResponseQueueCount"], 0);
    if (ResponseQueueCount <= 0) {
      ResponseQueueCount = ServerThreadCount / 10;
      if (ResponseQueueCount <= 0) ResponseQueueCount = 1;
    }
    Config::Bind(ServerGracefulShutdownWait, ini,
                 server["GracefulShutdownWait"], 0);
    Config::Bind(ServerHarshShutdown, ini, server["HarshShutdown"], true);
    Config::Bind(ServerKillOnSIGTERM, ini, server["KillOnSIGTERM"], false);
    Config::Bind(ServerEvilShutdown, ini, server["EvilShutdown"], true);
    Config::Bind(ServerDanglingWait, ini, server["DanglingWait"], 0);
    Config::Bind(ServerShutdownListenWait, ini, server["ShutdownListenWait"],
                 0);
    Config::Bind(ServerShutdownListenNoWork, ini,
                 server["ShutdownListenNoWork"], -1);
    Config::Get(ini, server["SSLNextProtocols"], ServerNextProtocols);
    if (ServerGracefulShutdownWait < ServerDanglingWait) {
      ServerGracefulShutdownWait = ServerDanglingWait;
    }
    Config::Bind(GzipCompressionLevel, ini, server["GzipCompressionLevel"], 3);

    Config::Bind(ForceCompressionURL, ini, server["ForceCompression"]["URL"]);
    Config::Bind(ForceCompressionCookie, ini,
                 server["ForceCompression"]["Cookie"]);
    Config::Bind(ForceCompressionParam, ini,
                 server["ForceCompression"]["Param"]);

    Config::Bind(EnableMagicQuotesGpc, ini, server["EnableMagicQuotesGpc"]);
    Config::Bind(EnableKeepAlive, ini, server["EnableKeepAlive"], true);
    Config::Bind(ExposeHPHP, ini, server["ExposeHPHP"], true);
    Config::Bind(ExposeXFBServer, ini, server["ExposeXFBServer"], false);
    Config::Bind(ExposeXFBDebug, ini, server["ExposeXFBDebug"], false);
    Config::Bind(XFBDebugSSLKey, ini, server["XFBDebugSSLKey"], "");
    Config::Bind(ConnectionTimeoutSeconds, ini,
                 server["ConnectionTimeoutSeconds"], -1);
    Config::Bind(EnableOutputBuffering, ini, server["EnableOutputBuffering"]);
    Config::Bind(OutputHandler, ini, server["OutputHandler"]);
    Config::Bind(ImplicitFlush, ini, server["ImplicitFlush"]);
    Config::Bind(EnableEarlyFlush, ini, server["EnableEarlyFlush"], true);
    Config::Bind(ForceChunkedEncoding, ini, server["ForceChunkedEncoding"]);
    Config::Bind(MaxPostSize, ini, server["MaxPostSize"], 100);
    MaxPostSize <<= 20;
    Config::Bind(AlwaysPopulateRawPostData, ini,
                 server["AlwaysPopulateRawPostData"], true);
    Config::Bind(LibEventSyncSend, ini, server["LibEventSyncSend"], true);
    Config::Bind(TakeoverFilename, ini, server["TakeoverFilename"]);
    Config::Bind(ExpiresActive, ini, server["ExpiresActive"], true);
    Config::Bind(ExpiresDefault, ini, server["ExpiresDefault"], 2592000);
    if (ExpiresDefault < 0) ExpiresDefault = 2592000;
    Config::Bind(DefaultCharsetName, ini, server["DefaultCharsetName"],
                 "utf-8");

    Config::Bind(RequestBodyReadLimit, ini, server["RequestBodyReadLimit"], -1);

    Config::Bind(EnableSSL, ini, server["EnableSSL"]);
    Config::Bind(SSLPort, ini, server["SSLPort"], 443);
    Config::Bind(SSLCertificateFile, ini, server["SSLCertificateFile"]);
    Config::Bind(SSLCertificateKeyFile, ini, server["SSLCertificateKeyFile"]);
    Config::Bind(SSLCertificateDir, ini, server["SSLCertificateDir"]);
    Config::Bind(TLSDisableTLS1_2, ini, server["TLSDisableTLS1_2"], false);
    Config::Bind(TLSClientCipherSpec, ini, server["TLSClientCipherSpec"]);

    string srcRoot = FileUtil::normalizeDir(
      Config::GetString(ini, server["SourceRoot"]));
    if (!srcRoot.empty()) SourceRoot = srcRoot;
    FileCache::SourceRoot = SourceRoot;

    Config::Get(ini, server["IncludeSearchPaths"], IncludeSearchPaths);
    for (unsigned int i = 0; i < IncludeSearchPaths.size(); i++) {
      IncludeSearchPaths[i] = FileUtil::normalizeDir(IncludeSearchPaths[i]);
    }
    IncludeSearchPaths.insert(IncludeSearchPaths.begin(), ".");

    Config::Bind(FileCache, ini, server["FileCache"]);
    Config::Bind(DefaultDocument, ini, server["DefaultDocument"]);
    Config::Bind(ErrorDocument404, ini, server["ErrorDocument404"]);
    normalizePath(ErrorDocument404);
    Config::Bind(ForbiddenAs404, ini, server["ForbiddenAs404"]);
    Config::Bind(ErrorDocument500, ini, server["ErrorDocument500"]);
    normalizePath(ErrorDocument500);
    Config::Bind(FatalErrorMessage, ini, server["FatalErrorMessage"]);
    FontPath = FileUtil::normalizeDir(
      Config::GetString(ini, server["FontPath"]));
    Config::Bind(EnableStaticContentFromDisk, ini,
                 server["EnableStaticContentFromDisk"], true);
    Config::Bind(EnableOnDemandUncompress, ini,
                 server["EnableOnDemandUncompress"], true);
    Config::Bind(EnableStaticContentMMap, ini,
                 server["EnableStaticContentMMap"], true);
    if (EnableStaticContentMMap) {
      EnableOnDemandUncompress = true;
    }
    Config::Bind(Utf8izeReplace, ini, server["Utf8izeReplace"], true);

    Config::Bind(StartupDocument, ini, server["StartupDocument"]);
    normalizePath(StartupDocument);
    Config::Bind(WarmupDocument, ini, server["WarmupDocument"]);
    Config::Bind(RequestInitFunction, ini, server["RequestInitFunction"]);
    Config::Bind(RequestInitDocument, ini, server["RequestInitDocument"]);

    Config::Bind(SafeFileAccess, ini, server["SafeFileAccess"]);
    Config::Get(ini, server["AllowedDirectories"], AllowedDirectories);

    Config::Bind(WhitelistExec, ini, server["WhitelistExec"]);
    Config::Bind(WhitelistExecWarningOnly, ini,
                 server["WhitelistExecWarningOnly"]);
    Config::Bind(AllowedExecCmds, ini, server["AllowedExecCmds"]);
    Config::Bind(UnserializationWhitelistCheck, ini,
                 server["UnserializationWhitelistCheck"], false);
    Config::Bind(UnserializationWhitelistCheckWarningOnly, ini,
                 server["UnserializationWhitelistCheckWarningOnly"], true);

    Config::Get(ini, server["AllowedFiles"], AllowedFiles);

    Config::Get(ini, server["ForbiddenFileExtensions"],
                ForbiddenFileExtensions);

    Config::Bind(LockCodeMemory, ini, server["LockCodeMemory"], false);
    Config::Bind(MaxArrayChain, ini, server["MaxArrayChain"], INT_MAX);
    if (MaxArrayChain != INT_MAX) {
      // MixedArray needs a higher threshold to avoid false-positives.
      // (and we always use MixedArray)
      MaxArrayChain *= 2;
    }

    Config::Bind(WarnOnCollectionToArray, ini,
                 server["WarnOnCollectionToArray"], false);
    Config::Bind(UseDirectCopy, ini, server["UseDirectCopy"], false);
    Config::Bind(AlwaysUseRelativePath, ini, server["AlwaysUseRelativePath"],
                 false);

    Hdf dns = server["DnsCache"];
    Config::Bind(EnableDnsCache, ini, dns["Enable"]);
    Config::Bind(DnsCacheTTL, ini, dns["TTL"], 600); // 10 minutes

    Hdf upload = server["Upload"];
    Config::Bind(UploadMaxFileSize, ini, upload["UploadMaxFileSize"], 100);
    UploadMaxFileSize <<= 20;

    Config::Bind(UploadTmpDir, ini, upload["UploadTmpDir"], "/tmp");
    Config::Bind(EnableFileUploads, ini, upload["EnableFileUploads"], true);
    Config::Bind(EnableUploadProgress, ini, upload["EnableUploadProgress"]);
    Config::Bind(Rfc1867Freq, ini, upload["Rfc1867Freq"], 256 * 1024);
    if (Rfc1867Freq < 0) Rfc1867Freq = 256 * 1024;
    Config::Bind(Rfc1867Prefix, ini, upload["Rfc1867Prefix"], "vupload_");
    Config::Bind(Rfc1867Name, ini, upload["Rfc1867Name"], "video_ptoken");

    Config::Bind(ImageMemoryMaxBytes, ini, server["ImageMemoryMaxBytes"], 0);
    if (ImageMemoryMaxBytes == 0) {
      ImageMemoryMaxBytes = UploadMaxFileSize * 2;
    }

    Config::Bind(LightProcessFilePrefix, ini, server["LightProcessFilePrefix"],
                 "./lightprocess");
    Config::Bind(LightProcessCount, ini, server["LightProcessCount"], 0);

    Config::Bind(ForceServerNameToHeader, ini,
                 server["ForceServerNameToHeader"]);

    Config::Bind(AllowDuplicateCookies, ini, server["AllowDuplicateCookies"],
                 !EnableHipHopSyntax);

    Config::Bind(EnableCufAsync, ini, server["EnableCufAsync"], false);
    Config::Bind(PathDebug, ini, server["PathDebug"], false);

    Config::Bind(ServerUser, ini, server["User"], "");
  }

  VirtualHost::SortAllowedDirectories(AllowedDirectories);
  {
    Hdf hosts = config["VirtualHost"];
    if (hosts.exists()) {
      for (Hdf hdf = hosts.firstChild(); hdf.exists(); hdf = hdf.next()) {
        if (hdf.getName() == "default") {
          VirtualHost::GetDefault().init(ini, hdf);
          VirtualHost::GetDefault().addAllowedDirectories(AllowedDirectories);
        } else {
          auto host = std::make_shared<VirtualHost>(ini, hdf);
          host->addAllowedDirectories(AllowedDirectories);
          VirtualHosts.push_back(host);
        }
      }
      for (unsigned int i = 0; i < VirtualHosts.size(); i++) {
        if (!VirtualHosts[i]->valid()) {
          throw std::runtime_error("virtual host missing prefix or pattern");
        }
      }
    }
  }
  {
    Hdf ipblocks = config["IpBlockMap"];
    IpBlocks = std::make_shared<IpBlockMap>(ini, ipblocks);
  }
  {
    Hdf satellites = config["Satellites"];
    if (satellites.exists()) {
      for (Hdf hdf = satellites.firstChild(); hdf.exists(); hdf = hdf.next()) {
        auto satellite = std::make_shared<SatelliteServerInfo>(ini, hdf);
        SatelliteServerInfos.push_back(satellite);
        if (satellite->getType() == SatelliteServer::Type::KindOfRPCServer) {
          XboxPassword = satellite->getPassword();
          XboxPasswords = satellite->getPasswords();
        }
      }
    }
  }
  {
    Hdf xbox = config["Xbox"];
    Config::Bind(XboxServerThreadCount, ini, xbox["ServerInfo.ThreadCount"],
                 10);
    Config::Bind(XboxServerMaxQueueLength, ini,
                 xbox["ServerInfo.MaxQueueLength"], INT_MAX);
    if (XboxServerMaxQueueLength < 0) XboxServerMaxQueueLength = INT_MAX;
    Config::Bind(XboxServerPort, ini, xbox["ServerInfo.Port"], 0);
    Config::Bind(XboxDefaultLocalTimeoutMilliSeconds, ini,
                 xbox["DefaultLocalTimeoutMilliSeconds"], 500);
    Config::Bind(XboxDefaultRemoteTimeoutSeconds, ini,
                 xbox["DefaultRemoteTimeoutSeconds"], 5);
    Config::Bind(XboxServerInfoMaxRequest, ini, xbox["ServerInfo.MaxRequest"],
                 500);
    Config::Bind(XboxServerInfoDuration, ini, xbox["ServerInfo.MaxDuration"],
                 120);
    Config::Bind(XboxServerInfoWarmupDoc, ini,
                 xbox["ServerInfo.WarmupDocument"], "");
    Config::Bind(XboxServerInfoReqInitFunc, ini,
                 xbox["ServerInfo.RequestInitFunction"], "");
    Config::Bind(XboxServerInfoReqInitDoc, ini,
                 xbox["ServerInfo.RequestInitDocument"], "");
    Config::Bind(XboxServerInfoAlwaysReset, ini,
                 xbox["ServerInfo.AlwaysReset"], false);
    Config::Bind(XboxServerLogInfo, ini, xbox["ServerInfo.LogInfo"], false);
    Config::Bind(XboxProcessMessageFunc, ini, xbox["ProcessMessageFunc"],
                 "xbox_process_message");
  }
  {
    Hdf pagelet = config["PageletServer"];
    Config::Bind(PageletServerThreadCount, ini, pagelet["ThreadCount"], 0);
    Config::Bind(PageletServerThreadRoundRobin, ini,
                 pagelet["ThreadRoundRobin"]);
    Config::Bind(PageletServerThreadDropStack, ini, pagelet["ThreadDropStack"]);
    Config::Bind(PageletServerThreadDropCacheTimeoutSeconds, ini,
                 pagelet["ThreadDropCacheTimeoutSeconds"], 0);
    Config::Bind(PageletServerQueueLimit, ini, pagelet["QueueLimit"], 0);
  }
  {
    Hdf content = config["StaticFile"];
    Config::Get(ini, content["Extensions"], StaticFileExtensions);
    Config::Get(ini, content["Generators"], StaticFileGenerators);

    Hdf matches = content["FilesMatch"];
    if (matches.exists()) {
      for (Hdf hdf = matches.firstChild(); hdf.exists(); hdf = hdf.next()) {
        FilesMatches.push_back(std::make_shared<FilesMatch>(ini, hdf));
      }
    }
  }
  {
    Hdf phpfile = config["PhpFile"];
    Config::Get(ini, phpfile["Extensions"], PhpFileExtensions);
  }
  {
    Hdf admin = config["AdminServer"];
    Config::Bind(AdminServerPort, ini, admin["Port"], 0);
    Config::Bind(AdminThreadCount, ini, admin["ThreadCount"], 1);
    Config::Bind(AdminPassword, ini, admin["Password"]);
    Config::Get(ini, admin["Passwords"], AdminPasswords);
  }
  {
    Hdf proxy = config["Proxy"];
    Config::Bind(ProxyOrigin, ini, proxy["Origin"]);
    Config::Bind(ProxyRetry, ini, proxy["Retry"], 3);
    Config::Bind(UseServeURLs, ini, proxy["ServeURLs"]);
    Config::Get(ini, proxy["ServeURLs"], ServeURLs);
    Config::Bind(UseProxyURLs, ini, proxy["ProxyURLs"]);
    Config::Bind(ProxyPercentage, ini, proxy["Percentage"], 0);
    Config::Get(ini, proxy["ProxyURLs"], ProxyURLs);
    Config::Get(ini, proxy["ProxyPatterns"], ProxyPatterns);
  }
  {
    Hdf http = config["Http"];
    Config::Bind(HttpDefaultTimeout, ini, http["DefaultTimeout"], 30);
    Config::Bind(HttpSlowQueryThreshold, ini, http["SlowQueryThreshold"], 5000);
  }
  {
    Hdf debug = config["Debug"];
    Config::Bind(NativeStackTrace, ini, debug["NativeStackTrace"]);
    StackTrace::Enabled = NativeStackTrace;
    Config::Bind(TranslateLeakStackTrace, ini,
                 debug["TranslateLeakStackTrace"]);
    Config::Bind(FullBacktrace, ini, debug["FullBacktrace"]);
    Config::Bind(ServerErrorMessage, ini, debug["ServerErrorMessage"]);
    Config::Bind(TranslateSource, ini, debug["TranslateSource"]);
    Config::Bind(RecordInput, ini, debug["RecordInput"]);
    Config::Bind(ClearInputOnSuccess, ini, debug["ClearInputOnSuccess"], true);
    Config::Bind(ProfilerOutputDir, ini, debug["ProfilerOutputDir"], "/tmp");
    Config::Bind(CoreDumpEmail, ini, debug["CoreDumpEmail"]);
    Config::Bind(CoreDumpReport, ini, debug["CoreDumpReport"], true);
    if (CoreDumpReport) {
      install_crash_reporter();
    }
    Config::Bind(CoreDumpReportDirectory, ini,
                 debug["CoreDumpReportDirectory"], CoreDumpReportDirectory);
    Config::Bind(LocalMemcache, ini, debug["LocalMemcache"]);
    Config::Bind(MemcacheReadOnly, ini, debug["MemcacheReadOnly"]);

    {
      Hdf simpleCounter = debug["SimpleCounter"];
      Config::Bind(SimpleCounter::SampleStackCount, ini,
                   simpleCounter["SampleStackCount"], 0);
      Config::Bind(SimpleCounter::SampleStackDepth, ini,
                   simpleCounter["SampleStackDepth"], 5);
    }
  }
  {
    Hdf stats = config["Stats"];
    Config::Bind(EnableStats, ini, stats); // main switch

    Config::Bind(EnableAPCStats, ini, stats["APC"], false);
    Config::Bind(EnableWebStats, ini, stats["Web"]);
    Config::Bind(EnableMemoryStats, ini, stats["Memory"]);
    Config::Bind(EnableMemcacheStats, ini, stats["Memcache"]);
    Config::Bind(EnableMemcacheKeyStats, ini, stats["MemcacheKey"]);
    Config::Bind(EnableSQLStats, ini, stats["SQL"]);
    Config::Bind(EnableSQLTableStats, ini, stats["SQLTable"]);
    Config::Bind(EnableNetworkIOStatus, ini, stats["NetworkIO"]);

    Config::Bind(StatsXSL, ini, stats["XSL"]);
    Config::Bind(StatsXSLProxy, ini, stats["XSLProxy"]);

    Config::Bind(StatsSlotDuration, ini, stats["SlotDuration"], 10 * 60);
    Config::Bind(StatsMaxSlot, ini, stats["MaxSlot"], 12 * 6); // 12 hours

    Config::Bind(EnableHotProfiler, ini, stats["EnableHotProfiler"], true);
    Config::Bind(ProfilerTraceBuffer, ini, stats["ProfilerTraceBuffer"],
                 2000000);
    Config::Bind(ProfilerTraceExpansion, ini, stats["ProfilerTraceExpansion"],
                 1.2);
    Config::Bind(ProfilerMaxTraceBuffer, ini, stats["ProfilerMaxTraceBuffer"],
                 0);
  }
  {
    Config::Get(ini, config["ServerVariables"], ServerVariables);
    Config::Get(ini, config["EnvVariables"], EnvVariables);
  }
  {
    Hdf sandbox = config["Sandbox"];
    Config::Bind(SandboxMode, ini, sandbox["SandboxMode"]);
    SandboxPattern = format_pattern(Config::GetString(ini, sandbox["Pattern"]),
                                    true);
    Config::Bind(SandboxHome, ini, sandbox["Home"]);
    Config::Bind(SandboxFallback, ini, sandbox["Fallback"]);
    Config::Bind(SandboxConfFile, ini, sandbox["ConfFile"]);
    Config::Bind(SandboxFromCommonRoot, ini, sandbox["FromCommonRoot"]);
    Config::Bind(SandboxDirectoriesRoot, ini, sandbox["DirectoriesRoot"]);
    Config::Bind(SandboxLogsRoot, ini, sandbox["LogsRoot"]);
    Config::Get(ini, sandbox["ServerVariables"], SandboxServerVariables);
  }
  {
    Hdf mail = config["Mail"];
    Config::Bind(SendmailPath, ini, mail["SendmailPath"], "sendmail -t -i");
    Config::Bind(MailForceExtraParameters, ini, mail["ForceExtraParameters"]);
  }
  {
    Hdf preg = config["Preg"];
    Config::Bind(PregBacktraceLimit, ini, preg["BacktraceLimit"], 1000000);
    Config::Bind(PregRecursionLimit, ini, preg["RecursionLimit"], 100000);
    Config::Bind(EnablePregErrorLog, ini, preg["ErrorLog"], true);
  }
  {
    Hdf hhprofServer = config["HHProfServer"];
    Config::Bind(HHProfServerEnabled, ini, hhprofServer["Enabled"], false);
    Config::Bind(HHProfServerPort, ini, hhprofServer["Port"], 4327);
    Config::Bind(HHProfServerThreads, ini, hhprofServer["Threads"], 2);
    Config::Bind(HHProfServerTimeoutSeconds, ini,
                 hhprofServer["TimeoutSeconds"], 30);
    Config::Bind(HHProfServerProfileClientMode, ini,
                 hhprofServer["ProfileClientMode"], true);
    Config::Bind(HHProfServerAllocationProfile, ini,
                 hhprofServer["AllocationProfile"], false);

    // HHProfServer.Filter.*
    Hdf hhprofFilter = hhprofServer["Filter"];
    Config::Bind(HHProfServerFilterMinAllocPerReq, ini,
                 hhprofFilter["MinAllocPerReq"], 2);
    Config::Bind(HHProfServerFilterMinBytesPerReq, ini,
                 hhprofFilter["MinBytesPerReq"], 128);
  }
  {
    Hdf simplexml = config["SimpleXML"];
    Config::Bind(SimpleXMLEmptyNamespaceMatchesAll, ini,
                 simplexml["EmptyNamespaceMatchesAll"], false);
  }
#ifdef FACEBOOK
  {
    Hdf fb303Server = config["Fb303Server"];
    Config::Bind(EnableFb303Server, ini, fb303Server["Enable"], true);
    Config::Bind(Fb303ServerPort, ini, fb303Server["Port"], 0);
    Config::Bind(Fb303ServerThreadStackSizeMb, ini,
                 fb303Server["ThreadStackSizeMb"], 8);
    Config::Bind(Fb303ServerWorkerThreads, ini, fb303Server["WorkerThreads"],
                 1);
    Config::Bind(Fb303ServerPoolThreads, ini, fb303Server["PoolThreads"], 1);
  }
#endif

  {
    Hdf hhprofServer = config["Xenon"];
    Config::Bind(XenonPeriodSeconds, ini, hhprofServer["Period"], 0.0);
    Config::Bind(XenonForceAlwaysOn, ini, hhprofServer["ForceAlwaysOn"], false);
  }

  Config::Get(ini, config["CustomSettings"], CustomSettings);

  if (RuntimeOption::OptionHooks != nullptr) {
    for (auto hookFunc : *RuntimeOption::OptionHooks) {
      hookFunc(ini, config);
    }
  }

  refineStaticStringTableSize();

  // Language and Misc Configuration Options
  IniSetting::Bind(IniSetting::CORE, IniSetting::PHP_INI_ONLY, "expose_php",
                   &RuntimeOption::ExposeHPHP);
  IniSetting::Bind(IniSetting::CORE, IniSetting::PHP_INI_PERDIR,
                   "auto_prepend_file", &RuntimeOption::AutoPrependFile);
  IniSetting::Bind(IniSetting::CORE, IniSetting::PHP_INI_PERDIR,
                   "auto_append_file", &RuntimeOption::AutoAppendFile);

  // Data Handling
  IniSetting::Bind(IniSetting::CORE, IniSetting::PHP_INI_PERDIR,
                   "post_max_size",
                   IniSetting::SetAndGet<int64_t>(
                     nullptr,
                     []() {
                       return VirtualHost::GetMaxPostSize();
                     }
                   ),
                   &RuntimeOption::MaxPostSize);
  IniSetting::Bind(IniSetting::CORE, IniSetting::PHP_INI_PERDIR,
                   "always_populate_raw_post_data",
                   &RuntimeOption::AlwaysPopulateRawPostData);

  // Paths and Directories
  IniSetting::Bind(IniSetting::CORE, IniSetting::PHP_INI_SYSTEM,
                   "doc_root", &RuntimeOption::SourceRoot);

  // FastCGI
  IniSetting::Bind(IniSetting::CORE, IniSetting::PHP_INI_ONLY,
                   "pid", &RuntimeOption::PidFile);

  // File Uploads
  IniSetting::Bind(IniSetting::CORE, IniSetting::PHP_INI_SYSTEM,
                   "file_uploads", "true",
                   &RuntimeOption::EnableFileUploads);
  IniSetting::Bind(IniSetting::CORE, IniSetting::PHP_INI_SYSTEM,
                   "upload_tmp_dir", &RuntimeOption::UploadTmpDir);
  IniSetting::Bind(IniSetting::CORE, IniSetting::PHP_INI_PERDIR,
                   "upload_max_filesize",
                   IniSetting::SetAndGet<std::string>(
                     [](const std::string& value) {
                       return ini_on_update(
                         value, RuntimeOption::UploadMaxFileSize);
                     },
                     []() {
                       int uploadMaxFilesize =
                         VirtualHost::GetUploadMaxFileSize() / (1 << 20);
                       return std::to_string(uploadMaxFilesize) + "M";
                     }
                   ));
  // Filesystem and Streams Configuration Options
  IniSetting::Bind(IniSetting::CORE, IniSetting::PHP_INI_SYSTEM,
                   "allow_url_fopen",
                   IniSetting::SetAndGet<std::string>(
                     [](const std::string& value) { return false; },
                     []() { return "1"; }));

  // HPHP specific
  IniSetting::Bind(IniSetting::CORE, IniSetting::PHP_INI_NONE,
                   "hphp.compiler_id",
                   IniSetting::SetAndGet<std::string>(
                     [](const std::string& value) { return false; },
                     []() { return getHphpCompilerId(); }
                   ));
  IniSetting::Bind(IniSetting::CORE, IniSetting::PHP_INI_NONE,
                   "hphp.compiler_version",
                   IniSetting::SetAndGet<std::string>(
                     [](const std::string& value) { return false; },
                     []() { return getHphpCompilerVersion(); }
                   ));
  IniSetting::Bind(IniSetting::CORE, IniSetting::PHP_INI_NONE,
                   "hphp.build_id",
                   IniSetting::SetAndGet<std::string>(
                     [](const std::string& value) { return false; },
                     nullptr
                   ),
                   &RuntimeOption::BuildId);
  IniSetting::Bind(IniSetting::CORE, IniSetting::PHP_INI_SYSTEM,
                   "notice_frequency",
                   &RuntimeOption::NoticeFrequency);
  IniSetting::Bind(IniSetting::CORE, IniSetting::PHP_INI_SYSTEM,
                   "warning_frequency",
                   &RuntimeOption::WarningFrequency);

  // Extensions
  Config::Bind(RuntimeOption::ExtensionDir, ini, "extension_dir",
               RuntimeOption::ExtensionDir);
  Config::Bind(RuntimeOption::DynamicExtensionPath, ini,
               config["DynamicExtensionPath"],
               RuntimeOption::DynamicExtensionPath);
  // there is no way to bind array ini/hdf settings.
  Config::Get(ini, config["extensions"], RuntimeOption::Extensions);
  Config::Get(ini, config["DynamicExtensions"],
              RuntimeOption::DynamicExtensions);


  Extension::LoadModules(ini, config);
  SharedStores::Create();
}

///////////////////////////////////////////////////////////////////////////////
}
