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
bool RuntimeOption::EnableChanneledJson = true;
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

static inline bool hhirRelaxGuardsDefault() {
  return RuntimeOption::EvalJitRegionSelector == "tracelet";
}

static inline bool hhbcRelaxGuardsDefault() {
  return !RuntimeOption::EvalHHIRRelaxGuards;
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

static void setResourceLimit(int resource, Hdf rlimit, const char *nodeName) {
  if (!Config::GetString(rlimit[nodeName]).empty()) {
    struct rlimit rl;
    getrlimit(resource, &rl);
    rl.rlim_cur = Config::GetInt64(rlimit[nodeName]);
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

static bool matchHdfPattern(const std::string &value, Hdf hdfPattern) {
  string pattern = Config::GetString(hdfPattern);
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

void RuntimeOption::Load(Hdf &config,
                         std::vector<std::string> *overwrites /* = NULL */,
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

    hostname = Config::GetString(machine["name"]);
    if (hostname.empty()) {
      hostname = Process::GetHostName();
    }

    tier = Config::GetString(machine["tier"]);

    cpu = Config::GetString(machine["cpu"]);
    if (cpu.empty()) {
      cpu = Process::GetCPUModel();
    }
  }

  // Tier overwrites
  {
    Hdf tiers = config["Tiers"];
    for (Hdf hdf = tiers.firstChild(); hdf.exists(); hdf = hdf.next()) {
      if (matchHdfPattern(hostname, hdf["machine"]) &&
          matchHdfPattern(tier, hdf["tier"]) &&
          matchHdfPattern(cpu, hdf["cpu"])) {
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

  PidFile = Config::GetString(config["PidFile"], "www.pid");

  Config::Get(config["DynamicInvokeFunctions"], DynamicInvokeFunctions);

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

    Logger::LogHeader = Config::GetBool(logger["Header"]);
    bool logInjectedStackTrace = Config::GetBool(logger["InjectedStackTrace"]);
    if (logInjectedStackTrace) {
      Logger::SetTheLogger(new ExtendedLogger());
      ExtendedLogger::EnabledByDefault = true;
    }
    Logger::LogNativeStackTrace = Config::GetBool(logger["NativeStackTrace"], true);
    Logger::MaxMessagesPerRequest =
      Config::GetInt32(logger["MaxMessagesPerRequest"], -1);

    Logger::UseSyslog = Config::GetBool(logger["UseSyslog"], false);
    Logger::UseLogFile = Config::GetBool(logger["UseLogFile"], true);
    IniSetting::Bind(IniSetting::CORE, IniSetting::PHP_INI_SYSTEM,
                     "hhvm.log.use_log_file", &Logger::UseLogFile);
    Logger::UseCronolog = Config::GetBool(logger["UseCronolog"], false);
    Logger::UseRequestLog = Config::GetBool(logger["UseRequestLog"], false);
    if (Logger::UseLogFile) {
      LogFile = Config::GetString(logger["File"]);
      if (!RuntimeOption::ServerExecutionMode()) {
        LogFile.clear();
      }
      if (LogFile[0] == '|') Logger::IsPipeOutput = true;
      LogFileSymLink = Config::GetString(logger["SymLink"]);
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
      Config::GetInt32(logger["DropCacheChunkSize"], 1 << 20);
    AlwaysEscapeLog = Config::GetBool(logger["AlwaysEscapeLog"], false);
    RuntimeOption::LogHeaderMangle = Config::GetInt32(logger["HeaderMangle"], 0);

    AlwaysLogUnhandledExceptions =
      Config::GetBool(logger["AlwaysLogUnhandledExceptions"], true);
    NoSilencer = Config::GetBool(logger["NoSilencer"]);
    RuntimeErrorReportingLevel =
      Config::GetInt32(logger["RuntimeErrorReportingLevel"],
                       static_cast<int>(ErrorConstants::ErrorModes::HPHP_ALL));

    AccessLogDefaultFormat = Config::GetString(logger["AccessLogDefaultFormat"],
                                               "%h %l %u %t \"%r\" %>s %b");
    {
      Hdf access = logger["Access"];
      for (Hdf hdf = access.firstChild(); hdf.exists();
           hdf = hdf.next()) {
        string fname = Config::GetString(hdf["File"]);
        if (fname.empty()) {
          continue;
        }
        string symLink = Config::GetString(hdf["SymLink"]);
        AccessLogs.push_back(AccessLogFileData(fname, symLink,
          Config::GetString(hdf["Format"], AccessLogDefaultFormat)));
      }
    }

    AdminLogFormat = Config::GetString(logger["AdminLog.Format"], "%h %t %s %U");
    AdminLogFile = Config::GetString(logger["AdminLog.File"]);
    AdminLogSymLink = Config::GetString(logger["AdminLog.SymLink"]);
  }
  {
    Hdf error = config["ErrorHandling"];

    /* Remove this, once its removed from production configs */
    (void)Config::GetBool(error["NoInfiniteLoopDetection"]);

    MaxSerializedStringSize =
      Config::GetInt32(error["MaxSerializedStringSize"], 64 * 1024 * 1024);
    CallUserHandlerOnFatals = Config::GetBool(error["CallUserHandlerOnFatals"], true);
    ThrowExceptionOnBadMethodCall =
      Config::GetBool(error["ThrowExceptionOnBadMethodCall"], true);
    MaxLoopCount = Config::GetInt32(error["MaxLoopCount"], 0);
    NoInfiniteRecursionDetection =
      Config::GetBool(error["NoInfiniteRecursionDetection"]);
    ThrowBadTypeExceptions = Config::GetBool(error["ThrowBadTypeExceptions"]);
    ThrowTooManyArguments = Config::GetBool(error["ThrowTooManyArguments"]);
    WarnTooManyArguments = Config::GetBool(error["WarnTooManyArguments"]);
    ThrowMissingArguments = Config::GetBool(error["ThrowMissingArguments"]);
    ThrowInvalidArguments = Config::GetBool(error["ThrowInvalidArguments"]);
    EnableHipHopErrors = Config::GetBool(error["EnableHipHopErrors"], true);
    AssertActive = Config::GetBool(error["AssertActive"]);
    AssertWarning = Config::GetBool(error["AssertWarning"]);
    NoticeFrequency = Config::GetInt64(error["NoticeFrequency"], 1);
    WarningFrequency = Config::GetInt64(error["WarningFrequency"], 1);
  }
  {
    Hdf rlimit = config["ResourceLimit"];
    if (Config::GetInt64(rlimit["CoreFileSizeOverride"])) {
      setResourceLimit(RLIMIT_CORE,   rlimit, "CoreFileSizeOverride");
    } else {
      setResourceLimit(RLIMIT_CORE,   rlimit, "CoreFileSize");
    }
    setResourceLimit(RLIMIT_NOFILE, rlimit, "MaxSocket");
    setResourceLimit(RLIMIT_DATA,   rlimit, "RSS");
    MaxRSS = Config::GetInt64(rlimit["MaxRSS"], 0);
    SocketDefaultTimeout = Config::GetInt64(rlimit["SocketDefaultTimeout"], 5);
    MaxRSSPollingCycle = Config::GetInt64(rlimit["MaxRSSPollingCycle"], 0);
    DropCacheCycle = Config::GetInt64(rlimit["DropCacheCycle"], 0);
    MaxSQLRowCount = Config::GetInt64(rlimit["MaxSQLRowCount"], 0);
    MaxMemcacheKeyCount = Config::GetInt64(rlimit["MaxMemcacheKeyCount"], 0);
    SerializationSizeLimit =
      Config::GetInt64(rlimit["SerializationSizeLimit"], StringData::MaxSize);
    StringOffsetLimit = Config::GetInt64(rlimit["StringOffsetLimit"], 10 * 1024 * 1024);
  }
  {
    Hdf server = config["Server"];
    Host = Config::GetString(server["Host"]);
    DefaultServerNameSuffix = Config::GetString(server["DefaultServerNameSuffix"]);
    ServerType = Config::GetString(server["Type"], ServerType);
    IniSetting::Bind(IniSetting::CORE, IniSetting::PHP_INI_SYSTEM,
                     "hhvm.server.type", &ServerType);
    ServerIP = Config::GetString(server["IP"]);
    ServerFileSocket = Config::GetString(server["FileSocket"]);
    IniSetting::Bind(IniSetting::CORE, IniSetting::PHP_INI_SYSTEM,
                     "hhvm.server.file_socket", &ServerFileSocket);
    ServerPrimaryIP = GetPrimaryIP();
    ServerPort = Config::GetUInt16(server["Port"], 80);
    IniSetting::Bind(IniSetting::CORE, IniSetting::PHP_INI_SYSTEM,
                     "hhvm.server.port", &ServerPort);
    ServerBacklog = Config::GetInt16(server["Backlog"], 128);
    ServerConnectionLimit = Config::GetInt16(server["ConnectionLimit"], 0);
    ServerThreadCount = Config::GetInt32(server["ThreadCount"], 50);
    ServerThreadRoundRobin = Config::GetBool(server["ThreadRoundRobin"]);
    ServerWarmupThrottleRequestCount =
      Config::GetInt32(server["WarmupThrottleRequestCount"],
                       ServerWarmupThrottleRequestCount);
    ServerThreadDropCacheTimeoutSeconds =
      Config::GetInt32(server["ThreadDropCacheTimeoutSeconds"], 0);
    if (Config::GetBool((server["ThreadJobLIFO"]))) {
      ServerThreadJobLIFOSwitchThreshold = 0;
    }
    ServerThreadJobLIFOSwitchThreshold =
      Config::GetInt32(server["ThreadJobLIFOSwitchThreshold"],
                       ServerThreadJobLIFOSwitchThreshold);
    ServerThreadJobMaxQueuingMilliSeconds =
      Config::GetInt16(server["ThreadJobMaxQueuingMilliSeconds"], -1);
    ServerThreadDropStack = Config::GetBool(server["ThreadDropStack"]);
    ServerHttpSafeMode = Config::GetBool(server["HttpSafeMode"]);
    ServerStatCache = Config::GetBool(server["StatCache"], false);
    Config::Get(server["WarmupRequests"], ServerWarmupRequests);
    Config::Get(server["HighPriorityEndPoints"], ServerHighPriorityEndPoints);
    ServerExitOnBindFail = Config::GetBool(server["ExitOnBindFail"], false);

    RequestTimeoutSeconds = Config::GetInt32(server["RequestTimeoutSeconds"], 0);
    PspTimeoutSeconds = Config::GetInt32(server["PspTimeoutSeconds"], 0);
    ServerMemoryHeadRoom = Config::GetInt64(server["MemoryHeadRoom"], 0);
    RequestMemoryMaxBytes = Config::GetInt64(server["RequestMemoryMaxBytes"], std::numeric_limits<int64_t>::max());
    ResponseQueueCount = Config::GetInt32(server["ResponseQueueCount"], 0);
    if (ResponseQueueCount <= 0) {
      ResponseQueueCount = ServerThreadCount / 10;
      if (ResponseQueueCount <= 0) ResponseQueueCount = 1;
    }
    ServerGracefulShutdownWait = Config::GetInt16(server["GracefulShutdownWait"], 0);
    ServerHarshShutdown = Config::GetBool(server["HarshShutdown"], true);
    ServerEvilShutdown = Config::GetBool(server["EvilShutdown"], true);
    ServerDanglingWait = Config::GetInt16(server["DanglingWait"], 0);
    ServerShutdownListenWait = Config::GetInt16(server["ShutdownListenWait"], 0);
    ServerShutdownListenNoWork = Config::GetInt16(server["ShutdownListenNoWork"], -1);
    Config::Get(server["SSLNextProtocols"], ServerNextProtocols);
    if (ServerGracefulShutdownWait < ServerDanglingWait) {
      ServerGracefulShutdownWait = ServerDanglingWait;
    }
    GzipCompressionLevel = Config::GetInt16(server["GzipCompressionLevel"], 3);
    EnableChanneledJson = Config::GetBool(server["EnableChanneledJson"], true);

    ForceCompressionURL    = Config::GetString(server["ForceCompression"]["URL"]);
    ForceCompressionCookie = Config::GetString(server["ForceCompression"]["Cookie"]);
    ForceCompressionParam  = Config::GetString(server["ForceCompression"]["Param"]);

    EnableMagicQuotesGpc = Config::GetBool(server["EnableMagicQuotesGpc"]);
    EnableKeepAlive = Config::GetBool(server["EnableKeepAlive"], true);
    ExposeHPHP = Config::GetBool(server["ExposeHPHP"], true);
    ExposeXFBServer = Config::GetBool(server["ExposeXFBServer"], false);
    ExposeXFBDebug = Config::GetBool(server["ExposeXFBDebug"], false);
    XFBDebugSSLKey = Config::GetString(server["XFBDebugSSLKey"], "");
    ConnectionTimeoutSeconds = Config::GetInt16(server["ConnectionTimeoutSeconds"], -1);
    EnableOutputBuffering = Config::GetBool(server["EnableOutputBuffering"]);
    OutputHandler = Config::GetString(server["OutputHandler"]);
    ImplicitFlush = Config::GetBool(server["ImplicitFlush"]);
    EnableEarlyFlush = Config::GetBool(server["EnableEarlyFlush"], true);
    ForceChunkedEncoding = Config::GetBool(server["ForceChunkedEncoding"]);
    MaxPostSize = Config::GetInt32(server["MaxPostSize"], 100) * (1LL << 20);
    AlwaysPopulateRawPostData =
      Config::GetBool(server["AlwaysPopulateRawPostData"], true);
    LibEventSyncSend = Config::GetBool(server["LibEventSyncSend"], true);
    TakeoverFilename = Config::GetString(server["TakeoverFilename"]);
    ExpiresActive = Config::GetBool(server["ExpiresActive"], true);
    ExpiresDefault = Config::GetInt32(server["ExpiresDefault"], 2592000);
    if (ExpiresDefault < 0) ExpiresDefault = 2592000;
    DefaultCharsetName = Config::GetString(server["DefaultCharsetName"], "utf-8");

    RequestBodyReadLimit = Config::GetInt32(server["RequestBodyReadLimit"], -1);

    EnableSSL = Config::GetBool(server["EnableSSL"]);
    SSLPort = Config::GetUInt16(server["SSLPort"], 443);
    SSLCertificateFile = Config::GetString(server["SSLCertificateFile"]);
    SSLCertificateKeyFile = Config::GetString(server["SSLCertificateKeyFile"]);
    SSLCertificateDir = Config::GetString(server["SSLCertificateDir"]);
    TLSDisableTLS1_2 = Config::GetBool(server["TLSDisableTLS1_2"], false);
    TLSClientCipherSpec = Config::GetString(server["TLSClientCipherSpec"]);

    string srcRoot = FileUtil::normalizeDir(Config::GetString(server["SourceRoot"]));
    if (!srcRoot.empty()) SourceRoot = srcRoot;
    FileCache::SourceRoot = SourceRoot;

    Config::Get(server["IncludeSearchPaths"], IncludeSearchPaths);
    for (unsigned int i = 0; i < IncludeSearchPaths.size(); i++) {
      IncludeSearchPaths[i] = FileUtil::normalizeDir(IncludeSearchPaths[i]);
    }
    IncludeSearchPaths.insert(IncludeSearchPaths.begin(), ".");

    FileCache = Config::GetString(server["FileCache"]);
    DefaultDocument = Config::GetString(server["DefaultDocument"]);
    IniSetting::Bind(IniSetting::CORE, IniSetting::PHP_INI_SYSTEM,
                     "hhvm.server.default_document", &DefaultDocument);
    ErrorDocument404 = Config::GetString(server["ErrorDocument404"]);
    normalizePath(ErrorDocument404);
    ForbiddenAs404 = Config::GetBool(server["ForbiddenAs404"]);
    ErrorDocument500 = Config::GetString(server["ErrorDocument500"]);
    normalizePath(ErrorDocument500);
    FatalErrorMessage = Config::GetString(server["FatalErrorMessage"]);
    FontPath = FileUtil::normalizeDir(Config::GetString(server["FontPath"]));
    EnableStaticContentFromDisk =
      Config::GetBool(server["EnableStaticContentFromDisk"], true);
    EnableOnDemandUncompress =
      Config::GetBool(server["EnableOnDemandUncompress"], true);
    EnableStaticContentMMap =
      Config::GetBool(server["EnableStaticContentMMap"], true);
    if (EnableStaticContentMMap) {
      EnableOnDemandUncompress = true;
    }
    Utf8izeReplace = Config::GetBool(server["Utf8izeReplace"], true);

    StartupDocument = Config::GetString(server["StartupDocument"]);
    normalizePath(StartupDocument);
    WarmupDocument = Config::GetString(server["WarmupDocument"]);
    RequestInitFunction = Config::GetString(server["RequestInitFunction"]);
    RequestInitDocument = Config::GetString(server["RequestInitDocument"]);
    Config::Get(server["ThreadDocuments"], ThreadDocuments);
    for (unsigned int i = 0; i < ThreadDocuments.size(); i++) {
      normalizePath(ThreadDocuments[i]);
    }
    Config::Get(server["ThreadLoopDocuments"], ThreadLoopDocuments);
    for (unsigned int i = 0; i < ThreadLoopDocuments.size(); i++) {
      normalizePath(ThreadLoopDocuments[i]);
    }

    SafeFileAccess = Config::GetBool(server["SafeFileAccess"]);
    Config::Get(server["AllowedDirectories"], AllowedDirectories);

    WhitelistExec = Config::GetBool(server["WhitelistExec"]);
    WhitelistExecWarningOnly = Config::GetBool(server["WhitelistExecWarningOnly"]);
    Config::Get(server["AllowedExecCmds"], AllowedExecCmds);

    UnserializationWhitelistCheck =
      Config::GetBool(server["UnserializationWhitelistCheck"], false);
    UnserializationWhitelistCheckWarningOnly =
      Config::GetBool(server["UnserializationWhitelistCheckWarningOnly"], true);

    Config::Get(server["AllowedFiles"], AllowedFiles);

    Config::Get(server["ForbiddenFileExtensions"], ForbiddenFileExtensions);

    LockCodeMemory = Config::GetBool(server["LockCodeMemory"], false);
    MaxArrayChain = Config::GetInt32(server["MaxArrayChain"], INT_MAX);
    if (MaxArrayChain != INT_MAX) {
      // MixedArray needs a higher threshold to avoid false-positives.
      // (and we always use MixedArray)
      MaxArrayChain *= 2;
    }

    WarnOnCollectionToArray = Config::GetBool(server["WarnOnCollectionToArray"], false);
    UseDirectCopy = Config::GetBool(server["UseDirectCopy"], false);
    AlwaysUseRelativePath = Config::GetBool(server["AlwaysUseRelativePath"], false);

    Hdf dns = server["DnsCache"];
    EnableDnsCache = Config::GetBool(dns["Enable"]);
    DnsCacheTTL = Config::GetInt32(dns["TTL"], 600); // 10 minutes
    DnsCacheKeyMaturityThreshold = Config::GetInt32(dns["KeyMaturityThreshold"], 20);
    DnsCacheMaximumCapacity = Config::GetInt64(dns["MaximumCapacity"], 0);
    DnsCacheKeyFrequencyUpdatePeriod = Config::GetInt32(dns["KeyFrequencyUpdatePeriod"], 1000);

    Hdf upload = server["Upload"];
    UploadMaxFileSize = Config::GetInt32(upload["UploadMaxFileSize"], 100)
      * (1LL << 20);

    UploadTmpDir = Config::GetString(upload["UploadTmpDir"], "/tmp");
    EnableFileUploads = Config::GetBool(upload["EnableFileUploads"], true);
    EnableUploadProgress = Config::GetBool(upload["EnableUploadProgress"]);
    Rfc1867Freq = Config::GetInt32(upload["Rfc1867Freq"], 256 * 1024);
    if (Rfc1867Freq < 0) Rfc1867Freq = 256 * 1024;
    Rfc1867Prefix = Config::GetString(upload["Rfc1867Prefix"], "vupload_");
    Rfc1867Name = Config::GetString(upload["Rfc1867Name"], "video_ptoken");

    ImageMemoryMaxBytes = Config::GetInt64(server["ImageMemoryMaxBytes"], 0);
    if (ImageMemoryMaxBytes == 0) {
      ImageMemoryMaxBytes = UploadMaxFileSize * 2;
    }

    LightProcessFilePrefix =
      Config::GetString(server["LightProcessFilePrefix"], "./lightprocess");
    LightProcessCount = Config::GetInt32(server["LightProcessCount"], 0);

    InjectedStackTrace = Config::GetBool(server["InjectedStackTrace"], true);
    InjectedStackTraceLimit = Config::GetInt32(server["InjectedStackTraceLimit"], -1);

    ForceServerNameToHeader = Config::GetBool(server["ForceServerNameToHeader"]);

    EnableCufAsync = Config::GetBool(server["EnableCufAsync"], false);
    PathDebug = Config::GetBool(server["PathDebug"], false);

    ServerUser = Config::GetString(server["User"], "");
  }

  VirtualHost::SortAllowedDirectories(AllowedDirectories);
  {
    Hdf hosts = config["VirtualHost"];
    if (hosts.exists()) {
      for (Hdf hdf = hosts.firstChild(); hdf.exists(); hdf = hdf.next()) {
        if (hdf.getName() == "default") {
          VirtualHost::GetDefault().init(hdf);
          VirtualHost::GetDefault().addAllowedDirectories(AllowedDirectories);
        } else {
          auto host = std::make_shared<VirtualHost>(hdf);
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
    IpBlocks = std::make_shared<IpBlockMap>(ipblocks);
  }
  {
    Hdf satellites = config["Satellites"];
    if (satellites.exists()) {
      for (Hdf hdf = satellites.firstChild(); hdf.exists(); hdf = hdf.next()) {
        auto satellite = std::make_shared<SatelliteServerInfo>(hdf);
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
    XboxServerThreadCount = Config::GetInt32(xbox["ServerInfo.ThreadCount"], 10);
    XboxServerMaxQueueLength =
      Config::GetInt32(xbox["ServerInfo.MaxQueueLength"], INT_MAX);
    if (XboxServerMaxQueueLength < 0) XboxServerMaxQueueLength = INT_MAX;
    XboxServerPort = Config::GetInt32(xbox["ServerInfo.Port"], 0);
    XboxDefaultLocalTimeoutMilliSeconds =
      Config::GetInt32(xbox["DefaultLocalTimeoutMilliSeconds"], 500);
    XboxDefaultRemoteTimeoutSeconds =
      Config::GetInt32(xbox["DefaultRemoteTimeoutSeconds"], 5);
    XboxServerInfoMaxRequest = Config::GetInt32(xbox["ServerInfo.MaxRequest"], 500);
    XboxServerInfoDuration = Config::GetInt32(xbox["ServerInfo.MaxDuration"], 120);
    XboxServerInfoWarmupDoc = Config::Get(xbox["ServerInfo.WarmupDocument"], "");
    XboxServerInfoReqInitFunc = Config::Get(xbox["ServerInfo.RequestInitFunction"], "");
    XboxServerInfoReqInitDoc = Config::Get(xbox["ServerInfo.RequestInitDocument"], "");
    XboxServerInfoAlwaysReset = Config::GetBool(xbox["ServerInfo.AlwaysReset"], false);
    XboxServerLogInfo = Config::GetBool(xbox["ServerInfo.LogInfo"], false);
    XboxProcessMessageFunc =
      Config::Get(xbox["ProcessMessageFunc"], "xbox_process_message");
  }
  {
    Hdf pagelet = config["PageletServer"];
    PageletServerThreadCount = Config::GetInt32(pagelet["ThreadCount"], 0);
    PageletServerThreadRoundRobin = Config::GetBool(pagelet["ThreadRoundRobin"]);
    PageletServerThreadDropStack = Config::GetBool(pagelet["ThreadDropStack"]);
    PageletServerThreadDropCacheTimeoutSeconds =
      Config::GetInt32(pagelet["ThreadDropCacheTimeoutSeconds"], 0);
    PageletServerQueueLimit = Config::GetInt32(pagelet["QueueLimit"], 0);
  }
  {
    FiberCount = Config::GetInt32(config["Fiber.ThreadCount"], Process::GetCPUCount());
  }
  {
    Hdf content = config["StaticFile"];
    Config::Get(content["Extensions"], StaticFileExtensions);
    Config::Get(content["Generators"], StaticFileGenerators);

    Hdf matches = content["FilesMatch"];
    if (matches.exists()) {
      for (Hdf hdf = matches.firstChild(); hdf.exists(); hdf = hdf.next()) {
        FilesMatches.push_back(std::make_shared<FilesMatch>(hdf));
      }
    }
  }
  {
    Hdf phpfile = config["PhpFile"];
    Config::Get(phpfile["Extensions"], PhpFileExtensions);
  }
  {
    Hdf admin = config["AdminServer"];
    AdminServerPort = Config::GetUInt16(admin["Port"], 0);
    AdminThreadCount = Config::GetInt32(admin["ThreadCount"], 1);
    AdminPassword = Config::GetString(admin["Password"]);
    Config::Get(admin["Passwords"], AdminPasswords);
  }
  {
    Hdf proxy = config["Proxy"];
    ProxyOrigin = Config::GetString(proxy["Origin"]);
    ProxyRetry = Config::GetInt16(proxy["Retry"], 3);
    UseServeURLs = Config::GetBool(proxy["ServeURLs"]);
    Config::Get(proxy["ServeURLs"], ServeURLs);
    UseProxyURLs = Config::GetBool(proxy["ProxyURLs"]);
    ProxyPercentage = Config::GetByte(proxy["Percentage"], 0);
    Config::Get(proxy["ProxyURLs"], ProxyURLs);
    Config::Get(proxy["ProxyPatterns"], ProxyPatterns);
  }
  {
    Hdf http = config["Http"];
    HttpDefaultTimeout = Config::GetInt32(http["DefaultTimeout"], 30);
    HttpSlowQueryThreshold = Config::GetInt32(http["SlowQueryThreshold"], 5000);
  }
  {
    Hdf debug = config["Debug"];
    NativeStackTrace = Config::GetBool(debug["NativeStackTrace"]);
    StackTrace::Enabled = NativeStackTrace;
    TranslateLeakStackTrace = Config::GetBool(debug["TranslateLeakStackTrace"]);
    FullBacktrace = Config::GetBool(debug["FullBacktrace"]);
    ServerStackTrace = Config::GetBool(debug["ServerStackTrace"]);
    ServerErrorMessage = Config::GetBool(debug["ServerErrorMessage"]);
    TranslateSource = Config::GetBool(debug["TranslateSource"]);
    RecordInput = Config::GetBool(debug["RecordInput"]);
    ClearInputOnSuccess = Config::GetBool(debug["ClearInputOnSuccess"], true);
    ProfilerOutputDir = Config::GetString(debug["ProfilerOutputDir"], "/tmp");
    CoreDumpEmail = Config::GetString(debug["CoreDumpEmail"]);
    CoreDumpReport = Config::GetBool(debug["CoreDumpReport"], true);
    if (CoreDumpReport) {
      install_crash_reporter();
    }
    CoreDumpReportDirectory =
      Config::GetString(debug["CoreDumpReportDirectory"], CoreDumpReportDirectory);
    LocalMemcache = Config::GetBool(debug["LocalMemcache"]);
    MemcacheReadOnly = Config::GetBool(debug["MemcacheReadOnly"]);

    {
      Hdf simpleCounter = debug["SimpleCounter"];
      SimpleCounter::SampleStackCount =
        Config::GetInt32(simpleCounter["SampleStackCount"], 0);
      SimpleCounter::SampleStackDepth =
        Config::GetInt32(simpleCounter["SampleStackDepth"], 5);
    }
  }
  {
    Hdf stats = config["Stats"];
    EnableStats = Config::GetBool(stats); // main switch

    EnableAPCStats = Config::GetBool(stats["APC"], false);
    EnableWebStats = Config::GetBool(stats["Web"]);
    EnableMemoryStats = Config::GetBool(stats["Memory"]);
    EnableMemcacheStats = Config::GetBool(stats["Memcache"]);
    EnableMemcacheKeyStats = Config::GetBool(stats["MemcacheKey"]);
    EnableSQLStats = Config::GetBool(stats["SQL"]);
    EnableSQLTableStats = Config::GetBool(stats["SQLTable"]);
    EnableNetworkIOStatus = Config::GetBool(stats["NetworkIO"]);

    StatsXSL = Config::GetString(stats["XSL"]);
    StatsXSLProxy = Config::GetString(stats["XSLProxy"]);

    StatsSlotDuration = Config::GetInt32(stats["SlotDuration"], 10 * 60); // 10 minutes
    StatsMaxSlot = Config::GetInt32(stats["MaxSlot"], 12 * 6); // 12 hours

    EnableHotProfiler = Config::GetBool(stats["EnableHotProfiler"], true);
    ProfilerTraceBuffer = Config::GetInt32(stats["ProfilerTraceBuffer"], 2000000);
    ProfilerTraceExpansion = Config::GetDouble(stats["ProfilerTraceExpansion"], 1.2);
    ProfilerMaxTraceBuffer = Config::GetInt32(stats["ProfilerMaxTraceBuffer"], 0);
  }
  {
    Config::Get(config["ServerVariables"], ServerVariables);
    Config::Get(config["EnvVariables"], EnvVariables);
  }
  {
    Hdf eval = config["Eval"];
    EnableHipHopSyntax = Config::GetBool(eval["EnableHipHopSyntax"]);
    EnableHipHopExperimentalSyntax =
      Config::GetBool(eval["EnableHipHopExperimentalSyntax"]);
    EnableShortTags= Config::GetBool(eval["EnableShortTags"], true);
    EnableAspTags = Config::GetBool(eval["EnableAspTags"]);
    EnableXHP = Config::GetBool(eval["EnableXHP"], false);
    IniSetting::Bind(IniSetting::CORE, IniSetting::PHP_INI_SYSTEM,
                     "hhvm.eval.enable_xhp", &EnableXHP);
    EnableZendCompat = Config::GetBool(eval["EnableZendCompat"], false);
    TimeoutsUseWallTime = Config::GetBool(eval["TimeoutsUseWallTime"], true);
    CheckFlushOnUserClose = Config::GetBool(eval["CheckFlushOnUserClose"], true);

    if (EnableHipHopSyntax) {
      // If EnableHipHopSyntax is true, it forces EnableXHP to true
      // regardless of how it was set in the config
      EnableXHP = true;
    }

    EnableObjDestructCall = Config::GetBool(eval["EnableObjDestructCall"], false);
    MaxUserFunctionId = Config::GetInt32(eval["MaxUserFunctionId"], 2 * 65536);
    CheckSymLink = Config::GetBool(eval["CheckSymLink"], true);

    EnableAlternative = Config::GetInt32(eval["EnableAlternative"], 0);

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
    Eval ## name = Config::get_ ##type(eval[#name], defaultVal);
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
    EnableEmitSwitch = Config::GetBool(eval["EnableEmitSwitch"], true);
    EnableEmitterStats = Config::GetBool(eval["EnableEmitterStats"], EnableEmitterStats);
    RecordCodeCoverage = Config::GetBool(eval["RecordCodeCoverage"]);
    if (EvalJit && RecordCodeCoverage) {
      throw InvalidArgumentException(
        "code coverage", "Code coverage is not supported for Eval.Jit=true");
    }
    if (RecordCodeCoverage) CheckSymLink = true;
    CodeCoverageOutputFile = Config::GetString(eval["CodeCoverageOutputFile"]);
    {
      Hdf debugger = eval["Debugger"];
      EnableDebugger = Config::GetBool(debugger["EnableDebugger"]);
      EnableDebuggerColor = Config::GetBool(debugger["EnableDebuggerColor"], true);
      EnableDebuggerPrompt = Config::GetBool(debugger["EnableDebuggerPrompt"], true);
      EnableDebuggerServer = Config::GetBool(debugger["EnableDebuggerServer"]);
      EnableDebuggerUsageLog = Config::GetBool(debugger["EnableDebuggerUsageLog"]);
      DebuggerServerPort = Config::GetUInt16(debugger["Port"], 8089);
      DebuggerDisableIPv6 = Config::GetBool(debugger["DisableIPv6"], false);
      DebuggerDefaultSandboxPath = Config::GetString(debugger["DefaultSandboxPath"]);
      DebuggerStartupDocument = Config::GetString(debugger["StartupDocument"]);
      DebuggerSignalTimeout = Config::GetInt32(debugger["SignalTimeout"], 1);

      DebuggerDefaultRpcPort = Config::GetUInt16(debugger["RPC.DefaultPort"], 8083);
      DebuggerDefaultRpcAuth = Config::GetString(debugger["RPC.DefaultAuth"]);
      DebuggerRpcHostDomain = Config::GetString(debugger["RPC.HostDomain"]);
      DebuggerDefaultRpcTimeout = Config::GetInt32(debugger["RPC.DefaultTimeout"], 30);
    }
    {
      Hdf lang = config["Hack"]["Lang"];
      IntsOverflowToInts =
        Config::GetBool(lang["IntsOverflowToInts"], EnableHipHopSyntax);
    }
    {
      Hdf repo = config["Repo"];
      {
        Hdf repoLocal = repo["Local"];
        // Repo.Local.Mode.
        RepoLocalMode = Config::GetString(repoLocal["Mode"]);
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
        RepoLocalPath = Config::GetString(repoLocal["Path"]);
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
        RepoCentralPath = Config::GetString(repoCentral["Path"]);
        IniSetting::Bind(IniSetting::CORE, IniSetting::PHP_INI_SYSTEM,
                         "hhvm.repo.central.path", &RepoCentralPath);
      }
      {
        Hdf repoEval = repo["Eval"];
        // Repo.Eval.Mode.
        RepoEvalMode = Config::GetString(repoEval["Mode"]);
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
      RepoJournal = Config::GetString(repo["Journal"], "delete");
      RepoCommit = Config::GetBool(repo["Commit"], true);
      RepoDebugInfo = Config::GetBool(repo["DebugInfo"], true);
      RepoAuthoritative = Config::GetBool(repo["Authoritative"], false);
    }

    // NB: after we know the value of RepoAuthoritative.
    EnableArgsInBacktraces =
      Config::GetBool(eval["EnableArgsInBacktraces"], !RepoAuthoritative);
    EvalAuthoritativeMode =
      Config::GetBool(eval["AuthoritativeMode"], false) || RepoAuthoritative;
  }
  {
    Hdf sandbox = config["Sandbox"];
    SandboxMode = Config::GetBool(sandbox["SandboxMode"]);
    SandboxPattern = format_pattern(Config::GetString(sandbox["Pattern"]), true);
    SandboxHome = Config::GetString(sandbox["Home"]);
    SandboxFallback = Config::GetString(sandbox["Fallback"]);
    SandboxConfFile = Config::GetString(sandbox["ConfFile"]);
    SandboxFromCommonRoot = Config::GetBool(sandbox["FromCommonRoot"]);
    SandboxDirectoriesRoot = Config::GetString(sandbox["DirectoriesRoot"]);
    SandboxLogsRoot = Config::GetString(sandbox["LogsRoot"]);
    Config::Get(sandbox["ServerVariables"], SandboxServerVariables);
  }
  {
    Hdf mail = config["Mail"];
    SendmailPath = Config::GetString(mail["SendmailPath"], "sendmail -t -i");
    MailForceExtraParameters = Config::GetString(mail["ForceExtraParameters"]);
  }
  {
    Hdf preg = config["Preg"];
    PregBacktraceLimit = Config::GetInt64(preg["BacktraceLimit"], 1000000);
    PregRecursionLimit = Config::GetInt64(preg["RecursionLimit"], 100000);
    EnablePregErrorLog = Config::GetBool(preg["ErrorLog"], true);
  }
  {
    Hdf hhprofServer = config["HHProfServer"];
    HHProfServerEnabled = Config::GetBool(hhprofServer["Enabled"], false);
    HHProfServerPort = Config::GetUInt16(hhprofServer["Port"], 4327);
    HHProfServerThreads = Config::GetInt16(hhprofServer["Threads"], 2);
    HHProfServerTimeoutSeconds =
      Config::GetInt64(hhprofServer["TimeoutSeconds"], 30);
    HHProfServerProfileClientMode =
      Config::GetBool(hhprofServer["ProfileClientMode"], true);
    HHProfServerAllocationProfile =
      Config::GetBool(hhprofServer["AllocationProfile"], false);

    // HHProfServer.Filter.*
    Hdf hhprofFilter = hhprofServer["Filter"];
    HHProfServerFilterMinAllocPerReq =
      Config::GetInt64(hhprofFilter["MinAllocPerReq"], 2);
    HHProfServerFilterMinBytesPerReq =
      Config::GetInt64(hhprofFilter["MinBytesPerReq"], 128);
  }
  {
    Hdf simplexml = config["SimpleXML"];
    SimpleXMLEmptyNamespaceMatchesAll =
      Config::GetBool(simplexml["EmptyNamespaceMatchesAll"], false);
  }
#ifdef FACEBOOK
  {
    Hdf fb303Server = config["Fb303Server"];
    EnableFb303Server = Config::GetBool(fb303Server["Enable"], true);
    Fb303ServerPort = Config::GetUInt16(fb303Server["Port"], 0);
    Fb303ServerThreadStackSizeMb = Config::GetInt16(fb303Server["ThreadStackSizeMb"], 8);
    Fb303ServerWorkerThreads = Config::GetInt16(fb303Server["WorkerThreads"], 1);
    Fb303ServerPoolThreads = Config::GetInt16(fb303Server["PoolThreads"], 1);
  }
#endif

  {
    Hdf hhprofServer = config["Xenon"];
    XenonPeriodSeconds = Config::GetDouble(hhprofServer["Period"], 0.0);
    XenonForceAlwaysOn = Config::GetBool(hhprofServer["ForceAlwaysOn"], false);
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

  Extension::LoadModules(config);
  SharedStores::Create();
  if (overwrites) Loaded = true;
}

///////////////////////////////////////////////////////////////////////////////
}
