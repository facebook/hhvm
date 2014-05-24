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

#include <sys/time.h>
#include <sys/resource.h>
#include <map>
#include <memory>
#include <set>
#include <vector>

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

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

bool RuntimeOption::Loaded = false;

const char *RuntimeOption::ExecutionMode = "";
std::string RuntimeOption::BuildId;
std::string RuntimeOption::InstanceId;
std::string RuntimeOption::PidFile = "www.pid";

std::string RuntimeOption::LogFile;
std::string RuntimeOption::LogFileSymLink;
int RuntimeOption::LogHeaderMangle = 0;
bool RuntimeOption::AlwaysEscapeLog = false;
bool RuntimeOption::AlwaysLogUnhandledExceptions = true;
bool RuntimeOption::InjectedStackTrace = true;
int RuntimeOption::InjectedStackTraceLimit = -1;
bool RuntimeOption::NoSilencer = false;
bool RuntimeOption::CallUserHandlerOnFatals = true;
bool RuntimeOption::ThrowExceptionOnBadMethodCall = true;
int RuntimeOption::RuntimeErrorReportingLevel =
  static_cast<int>(ErrorConstants::ErrorModes::HPHP_ALL);

std::string RuntimeOption::ServerUser;

int RuntimeOption::MaxLoopCount = 0;
int RuntimeOption::MaxSerializedStringSize = 64 * 1024 * 1024; // 64MB
bool RuntimeOption::NoInfiniteRecursionDetection = false;
bool RuntimeOption::ThrowBadTypeExceptions = false;
bool RuntimeOption::ThrowTooManyArguments = false;
bool RuntimeOption::WarnTooManyArguments = false;
bool RuntimeOption::ThrowMissingArguments = false;
bool RuntimeOption::ThrowInvalidArguments = false;
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
int RuntimeOption::FiberCount = Process::GetCPUCount();
int RuntimeOption::RequestTimeoutSeconds = 0;
int RuntimeOption::PspTimeoutSeconds = 0;
size_t RuntimeOption::ServerMemoryHeadRoom = 0;
int64_t RuntimeOption::RequestMemoryMaxBytes =
  std::numeric_limits<int64_t>::max();
int64_t RuntimeOption::ImageMemoryMaxBytes = 0;
int RuntimeOption::ResponseQueueCount = 0;
int RuntimeOption::ServerGracefulShutdownWait = 0;
bool RuntimeOption::ServerHarshShutdown = true;
bool RuntimeOption::ServerEvilShutdown = true;
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
std::vector<std::string> RuntimeOption::ThreadDocuments;
std::vector<std::string> RuntimeOption::ThreadLoopDocuments;

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
bool RuntimeOption::ServerStackTrace = false;
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
time_t RuntimeOption::DnsCacheKeyMaturityThreshold = 20;
size_t RuntimeOption::DnsCacheMaximumCapacity = 0;
int RuntimeOption::DnsCacheKeyFrequencyUpdatePeriod = 1000;

std::map<std::string, std::string> RuntimeOption::ServerVariables;
std::map<std::string, std::string> RuntimeOption::EnvVariables;

std::string RuntimeOption::LightProcessFilePrefix = "./lightprocess";
int RuntimeOption::LightProcessCount = 0;

bool RuntimeOption::EnableHipHopSyntax = false;
bool RuntimeOption::EnableHipHopExperimentalSyntax = false;
bool RuntimeOption::EnableShortTags = true;
bool RuntimeOption::EnableAspTags = false;
bool RuntimeOption::EnableXHP = false;
bool RuntimeOption::EnableObjDestructCall = false;
bool RuntimeOption::EnableEmitSwitch = true;
bool RuntimeOption::EnableEmitterStats = true;
bool RuntimeOption::CheckSymLink = true;
int RuntimeOption::MaxUserFunctionId = (2 * 65536);
bool RuntimeOption::EnableArgsInBacktraces = true;
bool RuntimeOption::EnableZendCompat = false;
bool RuntimeOption::TimeoutsUseWallTime = true;
bool RuntimeOption::CheckFlushOnUserClose = true;
bool RuntimeOption::EvalAuthoritativeMode = false;
bool RuntimeOption::IntsOverflowToInts = false;
HackStrictOption RuntimeOption::StrictArrayFillKeys = HackStrictOption::OFF,
  RuntimeOption::DisallowDynamicVarEnvFuncs = HackStrictOption::OFF;

int RuntimeOption::GetScannerType() {
  int type = 0;
  if (EnableShortTags) type |= Scanner::AllowShortTags;
  if (EnableAspTags) type |= Scanner::AllowAspTags;
  if (EnableXHP) type |= Scanner::AllowXHPSyntax;
  if (EnableHipHopSyntax) type |= Scanner::AllowHipHopSyntax;
  return type;
}

static inline std::string regionSelectorDefault() {
#ifdef HHVM_REGION_SELECTOR_TRACELET
  return "tracelet";
#else
#ifdef HHVM_REGION_SELECTOR_LEGACY
  return "legacy";
#else
  return "";
#endif
#endif
}

static inline bool hhirBytecodeControlFlowDefault() {
#ifdef HHVM_BYTECODE_CONTROL_FLOW
  return true;
#else
  return false;
#endif
}

static inline bool pgoDefault() {
  // TODO(3496304)
  return !RuntimeOption::EvalSimulateARM;
}

static inline bool evalJitDefault() {
#ifdef __APPLE__
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
static const int kDefaultWarmupRequests = debug ? 1 : 11;
static const int kDefaultJitPGOThreshold = debug ? 2 : 10;
static const uint32_t kDefaultProfileRequests = debug ? 1 << 31 : 500;
static const size_t kJitGlobalDataDef = RuntimeOption::EvalJitASize >> 2;
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

long RuntimeOption::PregBacktraceLimit = 1000000;
long RuntimeOption::PregRecursionLimit = 100000;
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
                         std::vector<std::string> *overwrites /* = nullptr */,
                         bool empty /* = false */) {
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

  PidFile = Config::GetString(ini, config["PidFile"], "www.pid");

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

    Logger::LogHeader = Config::GetBool(ini, logger["Header"]);
    bool logInjectedStackTrace = Config::GetBool(ini, logger["InjectedStackTrace"]);
    if (logInjectedStackTrace) {
      Logger::SetTheLogger(new ExtendedLogger());
      ExtendedLogger::EnabledByDefault = true;
    }
    Logger::LogNativeStackTrace = Config::GetBool(ini, logger["NativeStackTrace"], true);
    Logger::MaxMessagesPerRequest =
      Config::GetInt32(ini, logger["MaxMessagesPerRequest"], -1);

    Logger::UseSyslog = Config::GetBool(ini, logger["UseSyslog"], false);
    Logger::UseLogFile = Config::GetBool(ini, logger["UseLogFile"], true);
    IniSetting::Bind(IniSetting::CORE, IniSetting::PHP_INI_SYSTEM,
                     "hhvm.log.use_log_file", &Logger::UseLogFile);
    Logger::UseCronolog = Config::GetBool(ini, logger["UseCronolog"], false);
    Logger::UseRequestLog = Config::GetBool(ini, logger["UseRequestLog"], false);
    if (Logger::UseLogFile) {
      LogFile = Config::GetString(ini, logger["File"]);
      if (!RuntimeOption::ServerExecutionMode()) {
        LogFile.clear();
      }
      if (LogFile[0] == '|') Logger::IsPipeOutput = true;
      LogFileSymLink = Config::GetString(ini, logger["SymLink"]);
    }
    IniSetting::Bind(IniSetting::CORE, IniSetting::PHP_INI_SYSTEM,
                     "hhvm.log.file", IniSetting::SetAndGet<std::string>(
      [](const std::string& value) {
        LogFile = value;
        if (!RuntimeOption::ServerExecutionMode()) {
          LogFile.clear();
        }
        if (LogFile[0] == '|') Logger::IsPipeOutput = true;
        return true;
      }, []() {
        return LogFile;
      }
    ));
    LogFileFlusher::DropCacheChunkSize =
      Config::GetInt32(ini, logger["DropCacheChunkSize"], 1 << 20);
    AlwaysEscapeLog = Config::GetBool(ini, logger["AlwaysEscapeLog"], false);
    RuntimeOption::LogHeaderMangle = Config::GetInt32(ini, logger["HeaderMangle"], 0);

    AlwaysLogUnhandledExceptions =
      Config::GetBool(ini, logger["AlwaysLogUnhandledExceptions"], true);
    NoSilencer = Config::GetBool(ini, logger["NoSilencer"]);
    RuntimeErrorReportingLevel =
      Config::GetInt32(ini, logger["RuntimeErrorReportingLevel"],
                       static_cast<int>(ErrorConstants::ErrorModes::HPHP_ALL));

    AccessLogDefaultFormat = Config::GetString(ini, logger["AccessLogDefaultFormat"],
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

    AdminLogFormat = Config::GetString(ini, logger["AdminLog.Format"], "%h %t %s %U");
    AdminLogFile = Config::GetString(ini, logger["AdminLog.File"]);
    AdminLogSymLink = Config::GetString(ini, logger["AdminLog.SymLink"]);
  }
  {
    Hdf error = config["ErrorHandling"];

    /* Remove this, once its removed from production configs */
    (void)Config::GetBool(ini, error["NoInfiniteLoopDetection"]);

    MaxSerializedStringSize =
      Config::GetInt32(ini, error["MaxSerializedStringSize"], 64 * 1024 * 1024);
    CallUserHandlerOnFatals = Config::GetBool(ini, error["CallUserHandlerOnFatals"], true);
    ThrowExceptionOnBadMethodCall =
      Config::GetBool(ini, error["ThrowExceptionOnBadMethodCall"], true);
    MaxLoopCount = Config::GetInt32(ini, error["MaxLoopCount"], 0);
    NoInfiniteRecursionDetection =
      Config::GetBool(ini, error["NoInfiniteRecursionDetection"]);
    ThrowBadTypeExceptions = Config::GetBool(ini, error["ThrowBadTypeExceptions"]);
    ThrowTooManyArguments = Config::GetBool(ini, error["ThrowTooManyArguments"]);
    WarnTooManyArguments = Config::GetBool(ini, error["WarnTooManyArguments"]);
    ThrowMissingArguments = Config::GetBool(ini, error["ThrowMissingArguments"]);
    ThrowInvalidArguments = Config::GetBool(ini, error["ThrowInvalidArguments"]);
    EnableHipHopErrors = Config::GetBool(ini, error["EnableHipHopErrors"], true);
    AssertActive = Config::GetBool(ini, error["AssertActive"]);
    AssertWarning = Config::GetBool(ini, error["AssertWarning"]);
    NoticeFrequency = Config::GetInt64(ini, error["NoticeFrequency"], 1);
    WarningFrequency = Config::GetInt64(ini, error["WarningFrequency"], 1);
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
    MaxRSS = Config::GetInt64(ini, rlimit["MaxRSS"], 0);
    SocketDefaultTimeout = Config::GetInt64(ini, rlimit["SocketDefaultTimeout"], 5);
    MaxRSSPollingCycle = Config::GetInt64(ini, rlimit["MaxRSSPollingCycle"], 0);
    DropCacheCycle = Config::GetInt64(ini, rlimit["DropCacheCycle"], 0);
    MaxSQLRowCount = Config::GetInt64(ini, rlimit["MaxSQLRowCount"], 0);
    MaxMemcacheKeyCount = Config::GetInt64(ini, rlimit["MaxMemcacheKeyCount"], 0);
    SerializationSizeLimit =
      Config::GetInt64(ini, rlimit["SerializationSizeLimit"], StringData::MaxSize);
    StringOffsetLimit = Config::GetInt64(ini, rlimit["StringOffsetLimit"], 10 * 1024 * 1024);
  }
  {
    Hdf eval = config["Eval"];
    EnableHipHopSyntax = Config::GetBool(ini, eval["EnableHipHopSyntax"]);
    EnableHipHopExperimentalSyntax =
      Config::GetBool(ini, eval["EnableHipHopExperimentalSyntax"]);
    EnableShortTags= Config::GetBool(ini, eval["EnableShortTags"], true);
    EnableAspTags = Config::GetBool(ini, eval["EnableAspTags"]);
    EnableXHP = Config::GetBool(ini, eval["EnableXHP"], false);
    IniSetting::Bind(IniSetting::CORE, IniSetting::PHP_INI_SYSTEM,
                     "hhvm.eval.enable_xhp", &EnableXHP);
    EnableZendCompat = Config::GetBool(ini, eval["EnableZendCompat"], false);
    TimeoutsUseWallTime = Config::GetBool(ini, eval["TimeoutsUseWallTime"], true);
    CheckFlushOnUserClose = Config::GetBool(ini, eval["CheckFlushOnUserClose"], true);

    if (EnableHipHopSyntax) {
      // If EnableHipHopSyntax is true, it forces EnableXHP to true
      // regardless of how it was set in the config
      EnableXHP = true;
    }

    EnableObjDestructCall = Config::GetBool(ini, eval["EnableObjDestructCall"], false);
    MaxUserFunctionId = Config::GetInt32(ini, eval["MaxUserFunctionId"], 2 * 65536);
    CheckSymLink = Config::GetBool(ini, eval["CheckSymLink"], true);

    EnableAlternative = Config::GetInt32(ini, eval["EnableAlternative"], 0);

#define get_double GetDouble
#define get_bool GetBool
#define get_string GetString
#define get_int16 GetInt16
#define get_int32 GetInt32
#define get_int32_t GetInt32
#define get_int64 GetInt64
#define get_uint16 GetUInt16
#define get_uint32 GetUInt32
#define get_uint32_t GetUInt32
#define get_uint64 GetUInt64
#define get_uint64_t GetUInt64
#define F(type, name, defaultVal) \
    Eval ## name = Config::get_ ##type(ini, eval[#name], defaultVal);
    EVALFLAGS()
#undef F
#undef get_double
#undef get_bool
#undef get_string
#undef get_int16
#undef get_int32
#undef get_int64
#undef get_uint16
#undef get_uint32
#undef get_uint32_t
#undef get_uint64
    low_malloc_huge_pages(EvalMaxLowMemHugePages);
    EnableEmitSwitch = Config::GetBool(ini, eval["EnableEmitSwitch"], true);
    EnableEmitterStats = Config::GetBool(ini, eval["EnableEmitterStats"], EnableEmitterStats);
    RecordCodeCoverage = Config::GetBool(ini, eval["RecordCodeCoverage"]);
    if (EvalJit && RecordCodeCoverage) {
      throw InvalidArgumentException(
        "code coverage", "Code coverage is not supported for Eval.Jit=true");
    }
    if (RecordCodeCoverage) CheckSymLink = true;
    CodeCoverageOutputFile = Config::GetString(ini, eval["CodeCoverageOutputFile"]);
    {
      Hdf debugger = eval["Debugger"];
      EnableDebugger = Config::GetBool(ini, debugger["EnableDebugger"]);
      EnableDebuggerColor = Config::GetBool(ini, debugger["EnableDebuggerColor"], true);
      EnableDebuggerPrompt = Config::GetBool(ini, debugger["EnableDebuggerPrompt"], true);
      EnableDebuggerServer = Config::GetBool(ini, debugger["EnableDebuggerServer"]);
      EnableDebuggerUsageLog = Config::GetBool(ini, debugger["EnableDebuggerUsageLog"]);
      DebuggerServerPort = Config::GetUInt16(ini, debugger["Port"], 8089);
      DebuggerDisableIPv6 = Config::GetBool(ini, debugger["DisableIPv6"], false);
      DebuggerDefaultSandboxPath = Config::GetString(ini, debugger["DefaultSandboxPath"]);
      DebuggerStartupDocument = Config::GetString(ini, debugger["StartupDocument"]);
      DebuggerSignalTimeout = Config::GetInt32(ini, debugger["SignalTimeout"], 1);

      DebuggerDefaultRpcPort = Config::GetUInt16(ini, debugger["RPC.DefaultPort"], 8083);
      DebuggerDefaultRpcAuth = Config::GetString(ini, debugger["RPC.DefaultAuth"]);
      DebuggerRpcHostDomain = Config::GetString(ini, debugger["RPC.HostDomain"]);
      DebuggerDefaultRpcTimeout = Config::GetInt32(ini, debugger["RPC.DefaultTimeout"], 30);
    }
    {
      Hdf lang = config["Hack"]["Lang"];
      IntsOverflowToInts =
        Config::GetBool(ini, lang["IntsOverflowToInts"], EnableHipHopSyntax);
      StrictArrayFillKeys =
        Config::GetHackStrictOption(ini,
                                    lang["StrictArrayFillKeys"],
                                    EnableHipHopSyntax);
      DisallowDynamicVarEnvFuncs =
        Config::GetHackStrictOption(ini,
                                    lang["DisallowDynamicVarEnvFuncs"],
                                    EnableHipHopSyntax);
    }
    {
      Hdf repo = config["Repo"];
      {
        Hdf repoLocal = repo["Local"];
        // Repo.Local.Mode.
        RepoLocalMode = Config::GetString(ini, repoLocal["Mode"]);
        if (!empty && RepoLocalMode.empty()) {
          const char* HHVM_REPO_LOCAL_MODE = getenv("HHVM_REPO_LOCAL_MODE");
          if (HHVM_REPO_LOCAL_MODE != nullptr) {
            RepoLocalMode = HHVM_REPO_LOCAL_MODE;
          }
        }
        if (RepoLocalMode.empty()) {
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
        RepoLocalPath = Config::GetString(ini, repoLocal["Path"]);
        if (!empty && RepoLocalPath.empty()) {
          const char* HHVM_REPO_LOCAL_PATH = getenv("HHVM_REPO_LOCAL_PATH");
          if (HHVM_REPO_LOCAL_PATH != nullptr) {
            RepoLocalPath = HHVM_REPO_LOCAL_PATH;
          }
        }
      }
      {
        Hdf repoCentral = repo["Central"];
        // Repo.Central.Path.
        RepoCentralPath = Config::GetString(ini, repoCentral["Path"]);
        IniSetting::Bind(IniSetting::CORE, IniSetting::PHP_INI_SYSTEM,
                         "hhvm.repo.central.path", &RepoCentralPath);
      }
      {
        Hdf repoEval = repo["Eval"];
        // Repo.Eval.Mode.
        RepoEvalMode = Config::GetString(ini, repoEval["Mode"]);
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
      RepoJournal = Config::GetString(ini, repo["Journal"], "delete");
      RepoCommit = Config::GetBool(ini, repo["Commit"], true);
      RepoDebugInfo = Config::GetBool(ini, repo["DebugInfo"], true);
      RepoAuthoritative = Config::GetBool(ini, repo["Authoritative"], false);
    }

    // NB: after we know the value of RepoAuthoritative.
    EnableArgsInBacktraces =
      Config::GetBool(ini, eval["EnableArgsInBacktraces"], !RepoAuthoritative);
    EvalAuthoritativeMode =
      Config::GetBool(ini, eval["AuthoritativeMode"], false) || RepoAuthoritative;
  }
  {
    Hdf server = config["Server"];
    Host = Config::GetString(ini, server["Host"]);
    DefaultServerNameSuffix = Config::GetString(ini, server["DefaultServerNameSuffix"]);
    ServerType = Config::GetString(ini, server["Type"], ServerType);
    IniSetting::Bind(IniSetting::CORE, IniSetting::PHP_INI_SYSTEM,
                     "hhvm.server.type", &ServerType);
    ServerIP = Config::GetString(ini, server["IP"]);
    ServerFileSocket = Config::GetString(ini, server["FileSocket"]);
    IniSetting::Bind(IniSetting::CORE, IniSetting::PHP_INI_SYSTEM,
                     "hhvm.server.file_socket", &ServerFileSocket);
    ServerPrimaryIP = GetPrimaryIP();
    ServerPort = Config::GetUInt16(ini, server["Port"], 80);
    IniSetting::Bind(IniSetting::CORE, IniSetting::PHP_INI_SYSTEM,
                     "hhvm.server.port", &ServerPort);
    ServerBacklog = Config::GetInt16(ini, server["Backlog"], 128);
    ServerConnectionLimit = Config::GetInt16(ini, server["ConnectionLimit"], 0);
    ServerThreadCount = Config::GetInt32(ini, server["ThreadCount"], 50);
    ServerThreadRoundRobin = Config::GetBool(ini, server["ThreadRoundRobin"]);
    ServerWarmupThrottleRequestCount =
      Config::GetInt32(ini, server["WarmupThrottleRequestCount"],
                       ServerWarmupThrottleRequestCount);
    ServerThreadDropCacheTimeoutSeconds =
      Config::GetInt32(ini, server["ThreadDropCacheTimeoutSeconds"], 0);
    if (Config::GetBool(ini, server["ThreadJobLIFO"])) {
      ServerThreadJobLIFOSwitchThreshold = 0;
    }
    ServerThreadJobLIFOSwitchThreshold =
      Config::GetInt32(ini, server["ThreadJobLIFOSwitchThreshold"],
                       ServerThreadJobLIFOSwitchThreshold);
    ServerThreadJobMaxQueuingMilliSeconds =
      Config::GetInt16(ini, server["ThreadJobMaxQueuingMilliSeconds"], -1);
    ServerThreadDropStack = Config::GetBool(ini, server["ThreadDropStack"]);
    ServerHttpSafeMode = Config::GetBool(ini, server["HttpSafeMode"]);
    ServerStatCache = Config::GetBool(ini, server["StatCache"], false);
    ServerFixPathInfo = Config::GetBool(ini, server["FixPathInfo"], false);
    Config::Get(ini, server["WarmupRequests"], ServerWarmupRequests);
    Config::Get(ini, server["HighPriorityEndPoints"], ServerHighPriorityEndPoints);
    ServerExitOnBindFail = Config::GetBool(ini, server["ExitOnBindFail"], false);

    RequestTimeoutSeconds = Config::GetInt32(ini, server["RequestTimeoutSeconds"], 0);
    PspTimeoutSeconds = Config::GetInt32(ini, server["PspTimeoutSeconds"], 0);
    ServerMemoryHeadRoom = Config::GetInt64(ini, server["MemoryHeadRoom"], 0);
    RequestMemoryMaxBytes =
      Config::GetInt64(ini, server["RequestMemoryMaxBytes"],
                       std::numeric_limits<int64_t>::max());
    ResponseQueueCount = Config::GetInt32(ini, server["ResponseQueueCount"], 0);
    if (ResponseQueueCount <= 0) {
      ResponseQueueCount = ServerThreadCount / 10;
      if (ResponseQueueCount <= 0) ResponseQueueCount = 1;
    }
    ServerGracefulShutdownWait = Config::GetInt16(ini, server["GracefulShutdownWait"], 0);
    ServerHarshShutdown = Config::GetBool(ini, server["HarshShutdown"], true);
    ServerEvilShutdown = Config::GetBool(ini, server["EvilShutdown"], true);
    ServerDanglingWait = Config::GetInt16(ini, server["DanglingWait"], 0);
    ServerShutdownListenWait = Config::GetInt16(ini, server["ShutdownListenWait"], 0);
    ServerShutdownListenNoWork = Config::GetInt16(ini, server["ShutdownListenNoWork"], -1);
    Config::Get(ini, server["SSLNextProtocols"], ServerNextProtocols);
    if (ServerGracefulShutdownWait < ServerDanglingWait) {
      ServerGracefulShutdownWait = ServerDanglingWait;
    }
    GzipCompressionLevel = Config::GetInt16(ini, server["GzipCompressionLevel"], 3);

    ForceCompressionURL    = Config::GetString(ini, server["ForceCompression"]["URL"]);
    ForceCompressionCookie = Config::GetString(ini, server["ForceCompression"]["Cookie"]);
    ForceCompressionParam  = Config::GetString(ini, server["ForceCompression"]["Param"]);

    EnableMagicQuotesGpc = Config::GetBool(ini, server["EnableMagicQuotesGpc"]);
    EnableKeepAlive = Config::GetBool(ini, server["EnableKeepAlive"], true);
    ExposeHPHP = Config::GetBool(ini, server["ExposeHPHP"], true);
    ExposeXFBServer = Config::GetBool(ini, server["ExposeXFBServer"], false);
    ExposeXFBDebug = Config::GetBool(ini, server["ExposeXFBDebug"], false);
    XFBDebugSSLKey = Config::GetString(ini, server["XFBDebugSSLKey"], "");
    ConnectionTimeoutSeconds = Config::GetInt16(ini, server["ConnectionTimeoutSeconds"], -1);
    EnableOutputBuffering = Config::GetBool(ini, server["EnableOutputBuffering"]);
    OutputHandler = Config::GetString(ini, server["OutputHandler"]);
    ImplicitFlush = Config::GetBool(ini, server["ImplicitFlush"]);
    EnableEarlyFlush = Config::GetBool(ini, server["EnableEarlyFlush"], true);
    ForceChunkedEncoding = Config::GetBool(ini, server["ForceChunkedEncoding"]);
    MaxPostSize = Config::GetInt32(ini, server["MaxPostSize"], 100) * (1LL << 20);
    AlwaysPopulateRawPostData =
      Config::GetBool(ini, server["AlwaysPopulateRawPostData"], true);
    LibEventSyncSend = Config::GetBool(ini, server["LibEventSyncSend"], true);
    TakeoverFilename = Config::GetString(ini, server["TakeoverFilename"]);
    ExpiresActive = Config::GetBool(ini, server["ExpiresActive"], true);
    ExpiresDefault = Config::GetInt32(ini, server["ExpiresDefault"], 2592000);
    if (ExpiresDefault < 0) ExpiresDefault = 2592000;
    DefaultCharsetName = Config::GetString(ini, server["DefaultCharsetName"], "utf-8");

    RequestBodyReadLimit = Config::GetInt32(ini, server["RequestBodyReadLimit"], -1);

    EnableSSL = Config::GetBool(ini, server["EnableSSL"]);
    SSLPort = Config::GetUInt16(ini, server["SSLPort"], 443);
    SSLCertificateFile = Config::GetString(ini, server["SSLCertificateFile"]);
    SSLCertificateKeyFile = Config::GetString(ini, server["SSLCertificateKeyFile"]);
    SSLCertificateDir = Config::GetString(ini, server["SSLCertificateDir"]);
    TLSDisableTLS1_2 = Config::GetBool(ini, server["TLSDisableTLS1_2"], false);
    TLSClientCipherSpec = Config::GetString(ini, server["TLSClientCipherSpec"]);

    string srcRoot = FileUtil::normalizeDir(Config::GetString(ini, server["SourceRoot"]));
    if (!srcRoot.empty()) SourceRoot = srcRoot;
    FileCache::SourceRoot = SourceRoot;

    Config::Get(ini, server["IncludeSearchPaths"], IncludeSearchPaths);
    for (unsigned int i = 0; i < IncludeSearchPaths.size(); i++) {
      IncludeSearchPaths[i] = FileUtil::normalizeDir(IncludeSearchPaths[i]);
    }
    IncludeSearchPaths.insert(IncludeSearchPaths.begin(), ".");

    FileCache = Config::GetString(ini, server["FileCache"]);
    DefaultDocument = Config::GetString(ini, server["DefaultDocument"]);
    IniSetting::Bind(IniSetting::CORE, IniSetting::PHP_INI_SYSTEM,
                     "hhvm.server.default_document", &DefaultDocument);
    ErrorDocument404 = Config::GetString(ini, server["ErrorDocument404"]);
    normalizePath(ErrorDocument404);
    ForbiddenAs404 = Config::GetBool(ini, server["ForbiddenAs404"]);
    ErrorDocument500 = Config::GetString(ini, server["ErrorDocument500"]);
    normalizePath(ErrorDocument500);
    FatalErrorMessage = Config::GetString(ini, server["FatalErrorMessage"]);
    FontPath = FileUtil::normalizeDir(Config::GetString(ini, server["FontPath"]));
    EnableStaticContentFromDisk =
      Config::GetBool(ini, server["EnableStaticContentFromDisk"], true);
    EnableOnDemandUncompress =
      Config::GetBool(ini, server["EnableOnDemandUncompress"], true);
    EnableStaticContentMMap =
      Config::GetBool(ini, server["EnableStaticContentMMap"], true);
    if (EnableStaticContentMMap) {
      EnableOnDemandUncompress = true;
    }
    Utf8izeReplace = Config::GetBool(ini, server["Utf8izeReplace"], true);

    StartupDocument = Config::GetString(ini, server["StartupDocument"]);
    normalizePath(StartupDocument);
    WarmupDocument = Config::GetString(ini, server["WarmupDocument"]);
    RequestInitFunction = Config::GetString(ini, server["RequestInitFunction"]);
    RequestInitDocument = Config::GetString(ini, server["RequestInitDocument"]);
    Config::Get(ini, server["ThreadDocuments"], ThreadDocuments);
    for (unsigned int i = 0; i < ThreadDocuments.size(); i++) {
      normalizePath(ThreadDocuments[i]);
    }
    Config::Get(ini, server["ThreadLoopDocuments"], ThreadLoopDocuments);
    for (unsigned int i = 0; i < ThreadLoopDocuments.size(); i++) {
      normalizePath(ThreadLoopDocuments[i]);
    }

    SafeFileAccess = Config::GetBool(ini, server["SafeFileAccess"]);
    Config::Get(ini, server["AllowedDirectories"], AllowedDirectories);

    WhitelistExec = Config::GetBool(ini, server["WhitelistExec"]);
    WhitelistExecWarningOnly = Config::GetBool(ini, server["WhitelistExecWarningOnly"]);
    Config::Get(ini, server["AllowedExecCmds"], AllowedExecCmds);

    UnserializationWhitelistCheck =
      Config::GetBool(ini, server["UnserializationWhitelistCheck"], false);
    UnserializationWhitelistCheckWarningOnly =
      Config::GetBool(ini, server["UnserializationWhitelistCheckWarningOnly"], true);

    Config::Get(ini, server["AllowedFiles"], AllowedFiles);

    Config::Get(ini, server["ForbiddenFileExtensions"], ForbiddenFileExtensions);

    LockCodeMemory = Config::GetBool(ini, server["LockCodeMemory"], false);
    MaxArrayChain = Config::GetInt32(ini, server["MaxArrayChain"], INT_MAX);
    if (MaxArrayChain != INT_MAX) {
      // MixedArray needs a higher threshold to avoid false-positives.
      // (and we always use MixedArray)
      MaxArrayChain *= 2;
    }

    WarnOnCollectionToArray = Config::GetBool(ini, server["WarnOnCollectionToArray"], false);
    UseDirectCopy = Config::GetBool(ini, server["UseDirectCopy"], false);
    AlwaysUseRelativePath = Config::GetBool(ini, server["AlwaysUseRelativePath"], false);

    Hdf dns = server["DnsCache"];
    EnableDnsCache = Config::GetBool(ini, dns["Enable"]);
    DnsCacheTTL = Config::GetInt32(ini, dns["TTL"], 600); // 10 minutes
    DnsCacheKeyMaturityThreshold = Config::GetInt32(ini, dns["KeyMaturityThreshold"], 20);
    DnsCacheMaximumCapacity = Config::GetInt64(ini, dns["MaximumCapacity"], 0);
    DnsCacheKeyFrequencyUpdatePeriod = Config::GetInt32(ini, dns["KeyFrequencyUpdatePeriod"], 1000);

    Hdf upload = server["Upload"];
    UploadMaxFileSize = Config::GetInt32(ini, upload["UploadMaxFileSize"], 100)
      * (1LL << 20);

    UploadTmpDir = Config::GetString(ini, upload["UploadTmpDir"], "/tmp");
    EnableFileUploads = Config::GetBool(ini, upload["EnableFileUploads"], true);
    EnableUploadProgress = Config::GetBool(ini, upload["EnableUploadProgress"]);
    Rfc1867Freq = Config::GetInt32(ini, upload["Rfc1867Freq"], 256 * 1024);
    if (Rfc1867Freq < 0) Rfc1867Freq = 256 * 1024;
    Rfc1867Prefix = Config::GetString(ini, upload["Rfc1867Prefix"], "vupload_");
    Rfc1867Name = Config::GetString(ini, upload["Rfc1867Name"], "video_ptoken");

    ImageMemoryMaxBytes = Config::GetInt64(ini, server["ImageMemoryMaxBytes"], 0);
    if (ImageMemoryMaxBytes == 0) {
      ImageMemoryMaxBytes = UploadMaxFileSize * 2;
    }

    LightProcessFilePrefix =
      Config::GetString(ini, server["LightProcessFilePrefix"], "./lightprocess");
    LightProcessCount = Config::GetInt32(ini, server["LightProcessCount"], 0);

    InjectedStackTrace = Config::GetBool(ini, server["InjectedStackTrace"], true);
    InjectedStackTraceLimit = Config::GetInt32(ini, server["InjectedStackTraceLimit"], -1);

    ForceServerNameToHeader = Config::GetBool(ini, server["ForceServerNameToHeader"]);

    AllowDuplicateCookies =
      Config::GetBool(ini, server["AllowDuplicateCookies"], !EnableHipHopSyntax);

    EnableCufAsync = Config::GetBool(ini, server["EnableCufAsync"], false);
    PathDebug = Config::GetBool(ini, server["PathDebug"], false);

    ServerUser = Config::GetString(ini, server["User"], "");
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
          throw InvalidArgumentException("virtual host",
                                         "missing prefix or pattern");
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
    XboxServerThreadCount = Config::GetInt32(ini, xbox["ServerInfo.ThreadCount"], 10);
    XboxServerMaxQueueLength =
      Config::GetInt32(ini, xbox["ServerInfo.MaxQueueLength"], INT_MAX);
    if (XboxServerMaxQueueLength < 0) XboxServerMaxQueueLength = INT_MAX;
    XboxServerPort = Config::GetInt32(ini, xbox["ServerInfo.Port"], 0);
    XboxDefaultLocalTimeoutMilliSeconds =
      Config::GetInt32(ini, xbox["DefaultLocalTimeoutMilliSeconds"], 500);
    XboxDefaultRemoteTimeoutSeconds =
      Config::GetInt32(ini, xbox["DefaultRemoteTimeoutSeconds"], 5);
    XboxServerInfoMaxRequest = Config::GetInt32(ini, xbox["ServerInfo.MaxRequest"], 500);
    XboxServerInfoDuration = Config::GetInt32(ini, xbox["ServerInfo.MaxDuration"], 120);
    XboxServerInfoWarmupDoc = Config::Get(ini, xbox["ServerInfo.WarmupDocument"], "");
    XboxServerInfoReqInitFunc = Config::Get(ini, xbox["ServerInfo.RequestInitFunction"], "");
    XboxServerInfoReqInitDoc = Config::Get(ini, xbox["ServerInfo.RequestInitDocument"], "");
    XboxServerInfoAlwaysReset = Config::GetBool(ini, xbox["ServerInfo.AlwaysReset"], false);
    XboxServerLogInfo = Config::GetBool(ini, xbox["ServerInfo.LogInfo"], false);
    XboxProcessMessageFunc =
      Config::Get(ini, xbox["ProcessMessageFunc"], "xbox_process_message");
  }
  {
    Hdf pagelet = config["PageletServer"];
    PageletServerThreadCount = Config::GetInt32(ini, pagelet["ThreadCount"], 0);
    PageletServerThreadRoundRobin = Config::GetBool(ini, pagelet["ThreadRoundRobin"]);
    PageletServerThreadDropStack = Config::GetBool(ini, pagelet["ThreadDropStack"]);
    PageletServerThreadDropCacheTimeoutSeconds =
      Config::GetInt32(ini, pagelet["ThreadDropCacheTimeoutSeconds"], 0);
    PageletServerQueueLimit = Config::GetInt32(ini, pagelet["QueueLimit"], 0);
  }
  {
    FiberCount = Config::GetInt32(ini, config["Fiber.ThreadCount"], Process::GetCPUCount());
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
    AdminServerPort = Config::GetUInt16(ini, admin["Port"], 0);
    AdminThreadCount = Config::GetInt32(ini, admin["ThreadCount"], 1);
    AdminPassword = Config::GetString(ini, admin["Password"]);
    Config::Get(ini, admin["Passwords"], AdminPasswords);
  }
  {
    Hdf proxy = config["Proxy"];
    ProxyOrigin = Config::GetString(ini, proxy["Origin"]);
    ProxyRetry = Config::GetInt16(ini, proxy["Retry"], 3);
    UseServeURLs = Config::GetBool(ini, proxy["ServeURLs"]);
    Config::Get(ini, proxy["ServeURLs"], ServeURLs);
    UseProxyURLs = Config::GetBool(ini, proxy["ProxyURLs"]);
    ProxyPercentage = Config::GetByte(ini, proxy["Percentage"], 0);
    Config::Get(ini, proxy["ProxyURLs"], ProxyURLs);
    Config::Get(ini, proxy["ProxyPatterns"], ProxyPatterns);
  }
  {
    Hdf http = config["Http"];
    HttpDefaultTimeout = Config::GetInt32(ini, http["DefaultTimeout"], 30);
    HttpSlowQueryThreshold = Config::GetInt32(ini, http["SlowQueryThreshold"], 5000);
  }
  {
    Hdf debug = config["Debug"];
    NativeStackTrace = Config::GetBool(ini, debug["NativeStackTrace"]);
    StackTrace::Enabled = NativeStackTrace;
    TranslateLeakStackTrace = Config::GetBool(ini, debug["TranslateLeakStackTrace"]);
    FullBacktrace = Config::GetBool(ini, debug["FullBacktrace"]);
    ServerStackTrace = Config::GetBool(ini, debug["ServerStackTrace"]);
    ServerErrorMessage = Config::GetBool(ini, debug["ServerErrorMessage"]);
    TranslateSource = Config::GetBool(ini, debug["TranslateSource"]);
    RecordInput = Config::GetBool(ini, debug["RecordInput"]);
    ClearInputOnSuccess = Config::GetBool(ini, debug["ClearInputOnSuccess"], true);
    ProfilerOutputDir = Config::GetString(ini, debug["ProfilerOutputDir"], "/tmp");
    CoreDumpEmail = Config::GetString(ini, debug["CoreDumpEmail"]);
    CoreDumpReport = Config::GetBool(ini, debug["CoreDumpReport"], true);
    if (CoreDumpReport) {
      install_crash_reporter();
    }
    CoreDumpReportDirectory =
      Config::GetString(ini, debug["CoreDumpReportDirectory"], CoreDumpReportDirectory);
    LocalMemcache = Config::GetBool(ini, debug["LocalMemcache"]);
    MemcacheReadOnly = Config::GetBool(ini, debug["MemcacheReadOnly"]);

    {
      Hdf simpleCounter = debug["SimpleCounter"];
      SimpleCounter::SampleStackCount =
        Config::GetInt32(ini, simpleCounter["SampleStackCount"], 0);
      SimpleCounter::SampleStackDepth =
        Config::GetInt32(ini, simpleCounter["SampleStackDepth"], 5);
    }
  }
  {
    Hdf stats = config["Stats"];
    EnableStats = Config::GetBool(ini, stats); // main switch

    EnableAPCStats = Config::GetBool(ini, stats["APC"], false);
    EnableWebStats = Config::GetBool(ini, stats["Web"]);
    EnableMemoryStats = Config::GetBool(ini, stats["Memory"]);
    EnableMemcacheStats = Config::GetBool(ini, stats["Memcache"]);
    EnableMemcacheKeyStats = Config::GetBool(ini, stats["MemcacheKey"]);
    EnableSQLStats = Config::GetBool(ini, stats["SQL"]);
    EnableSQLTableStats = Config::GetBool(ini, stats["SQLTable"]);
    EnableNetworkIOStatus = Config::GetBool(ini, stats["NetworkIO"]);

    StatsXSL = Config::GetString(ini, stats["XSL"]);
    StatsXSLProxy = Config::GetString(ini, stats["XSLProxy"]);

    StatsSlotDuration = Config::GetInt32(ini, stats["SlotDuration"], 10 * 60); // 10 minutes
    StatsMaxSlot = Config::GetInt32(ini, stats["MaxSlot"], 12 * 6); // 12 hours

    EnableHotProfiler = Config::GetBool(ini, stats["EnableHotProfiler"], true);
    ProfilerTraceBuffer = Config::GetInt32(ini, stats["ProfilerTraceBuffer"], 2000000);
    ProfilerTraceExpansion = Config::GetDouble(ini, stats["ProfilerTraceExpansion"], 1.2);
    ProfilerMaxTraceBuffer = Config::GetInt32(ini, stats["ProfilerMaxTraceBuffer"], 0);
  }
  {
    Config::Get(ini, config["ServerVariables"], ServerVariables);
    Config::Get(ini, config["EnvVariables"], EnvVariables);
  }
  {
    Hdf sandbox = config["Sandbox"];
    SandboxMode = Config::GetBool(ini, sandbox["SandboxMode"]);
    SandboxPattern = format_pattern(Config::GetString(ini, sandbox["Pattern"]), true);
    SandboxHome = Config::GetString(ini, sandbox["Home"]);
    SandboxFallback = Config::GetString(ini, sandbox["Fallback"]);
    SandboxConfFile = Config::GetString(ini, sandbox["ConfFile"]);
    SandboxFromCommonRoot = Config::GetBool(ini, sandbox["FromCommonRoot"]);
    SandboxDirectoriesRoot = Config::GetString(ini, sandbox["DirectoriesRoot"]);
    SandboxLogsRoot = Config::GetString(ini, sandbox["LogsRoot"]);
    Config::Get(ini, sandbox["ServerVariables"], SandboxServerVariables);
  }
  {
    Hdf mail = config["Mail"];
    SendmailPath = Config::GetString(ini, mail["SendmailPath"], "sendmail -t -i");
    MailForceExtraParameters = Config::GetString(ini, mail["ForceExtraParameters"]);
  }
  {
    Hdf preg = config["Preg"];
    PregBacktraceLimit = Config::GetInt64(ini, preg["BacktraceLimit"], 1000000);
    PregRecursionLimit = Config::GetInt64(ini, preg["RecursionLimit"], 100000);
    EnablePregErrorLog = Config::GetBool(ini, preg["ErrorLog"], true);
  }
  {
    Hdf hhprofServer = config["HHProfServer"];
    HHProfServerEnabled = Config::GetBool(ini, hhprofServer["Enabled"], false);
    HHProfServerPort = Config::GetUInt16(ini, hhprofServer["Port"], 4327);
    HHProfServerThreads = Config::GetInt16(ini, hhprofServer["Threads"], 2);
    HHProfServerTimeoutSeconds =
      Config::GetInt64(ini, hhprofServer["TimeoutSeconds"], 30);
    HHProfServerProfileClientMode =
      Config::GetBool(ini, hhprofServer["ProfileClientMode"], true);
    HHProfServerAllocationProfile =
      Config::GetBool(ini, hhprofServer["AllocationProfile"], false);

    // HHProfServer.Filter.*
    Hdf hhprofFilter = hhprofServer["Filter"];
    HHProfServerFilterMinAllocPerReq =
      Config::GetInt64(ini, hhprofFilter["MinAllocPerReq"], 2);
    HHProfServerFilterMinBytesPerReq =
      Config::GetInt64(ini, hhprofFilter["MinBytesPerReq"], 128);
  }
  {
    Hdf simplexml = config["SimpleXML"];
    SimpleXMLEmptyNamespaceMatchesAll =
      Config::GetBool(ini, simplexml["EmptyNamespaceMatchesAll"], false);
  }
#ifdef FACEBOOK
  {
    Hdf fb303Server = config["Fb303Server"];
    EnableFb303Server = Config::GetBool(ini, fb303Server["Enable"], true);
    Fb303ServerPort = Config::GetUInt16(ini, fb303Server["Port"], 0);
    Fb303ServerThreadStackSizeMb = Config::GetInt16(ini, fb303Server["ThreadStackSizeMb"], 8);
    Fb303ServerWorkerThreads = Config::GetInt16(ini, fb303Server["WorkerThreads"], 1);
    Fb303ServerPoolThreads = Config::GetInt16(ini, fb303Server["PoolThreads"], 1);
  }
#endif

  {
    Hdf hhprofServer = config["Xenon"];
    XenonPeriodSeconds = Config::GetDouble(ini, hhprofServer["Period"], 0.0);
    XenonForceAlwaysOn = Config::GetBool(ini, hhprofServer["ForceAlwaysOn"], false);
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
  IniSetting::Bind(IniSetting::CORE, IniSetting::PHP_INI_ONLY,
                   "hhvm.eval.jit", &EvalJit);
  IniSetting::Bind(IniSetting::CORE, IniSetting::PHP_INI_ONLY,
                   "hhvm.eval.jit_pseudomain", &EvalJitPseudomain);
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
                   "hhvm.ext_zend_compat",
                   IniSetting::SetAndGet<bool>(
                     [](const bool& value) { return false; },
                     nullptr
                   ),
                   &RuntimeOption::EnableZendCompat),
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

  Extension::LoadModules(ini, config);
  SharedStores::Create();
  if (overwrites) Loaded = true;
}

///////////////////////////////////////////////////////////////////////////////
}
