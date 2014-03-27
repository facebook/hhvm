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
#include "hphp/util/file-util.h"

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
bool RuntimeOption::EnableInstructionCounts = false;
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

static inline bool hhirRefcountOptsDefault() {
  // TODO(t3091846)
  // TODO(t3728863)
  return !RuntimeOption::EvalSimulateARM &&
    !RuntimeOption::EvalHHIRBytecodeControlFlow;
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

int RuntimeOption::EnableAlternative = 0;

///////////////////////////////////////////////////////////////////////////////

static void setResourceLimit(int resource, Hdf rlimit, const char *nodeName) {
  if (!rlimit[nodeName].getString().empty()) {
    struct rlimit rl;
    getrlimit(resource, &rl);
    rl.rlim_cur = rlimit[nodeName].getInt64();
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
  string pattern = hdfPattern.getString();
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
  // Machine metrics
  string hostname, tier, cpu;
  {
    Hdf machine = config["Machine"];

    hostname = machine["name"].getString();
    if (hostname.empty()) {
      hostname = Process::GetHostName();
    }

    tier = machine["tier"].getString();

    cpu = machine["cpu"].getString();
    if (cpu.empty()) {
      cpu = Process::GetCPUModel();
    }
  }

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

  PidFile = config["PidFile"].getString("www.pid");

  config["DynamicInvokeFunctions"].get(DynamicInvokeFunctions);

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

    Logger::LogHeader = logger["Header"].getBool();
    bool logInjectedStackTrace = logger["InjectedStackTrace"].getBool();
    if (logInjectedStackTrace) {
      Logger::SetTheLogger(new ExtendedLogger());
      ExtendedLogger::EnabledByDefault = true;
    }
    Logger::LogNativeStackTrace = logger["NativeStackTrace"].getBool(true);
    Logger::MaxMessagesPerRequest =
      logger["MaxMessagesPerRequest"].getInt32(-1);

    Logger::UseSyslog = logger["UseSyslog"].getBool(false);
    Logger::UseLogFile = logger["UseLogFile"].getBool(true);
    IniSetting::Bind(IniSetting::CORE, IniSetting::PHP_INI_SYSTEM,
                     "hhvm.log.use_log_file", &Logger::UseLogFile);
    Logger::UseCronolog = logger["UseCronolog"].getBool(false);
    Logger::UseRequestLog = logger["UseRequestLog"].getBool(false);
    if (Logger::UseLogFile) {
      LogFile = logger["File"].getString();
      if (!RuntimeOption::ServerExecutionMode()) {
        LogFile.clear();
      }
      if (LogFile[0] == '|') Logger::IsPipeOutput = true;
      LogFileSymLink = logger["SymLink"].getString();
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
      logger["DropCacheChunkSize"].getInt32(1 << 20);
    AlwaysEscapeLog = logger["AlwaysEscapeLog"].getBool(false);
    RuntimeOption::LogHeaderMangle = logger["HeaderMangle"].getInt32(0);

    AlwaysLogUnhandledExceptions =
      logger["AlwaysLogUnhandledExceptions"].getBool(true);
    NoSilencer = logger["NoSilencer"].getBool();
    RuntimeErrorReportingLevel =
      logger["RuntimeErrorReportingLevel"]
        .getInt32(static_cast<int>(ErrorConstants::ErrorModes::HPHP_ALL));

    AccessLogDefaultFormat = logger["AccessLogDefaultFormat"].
      getString("%h %l %u %t \"%r\" %>s %b");
    {
      Hdf access = logger["Access"];
      for (Hdf hdf = access.firstChild(); hdf.exists();
           hdf = hdf.next()) {
        string fname = hdf["File"].getString();
        if (fname.empty()) {
          continue;
        }
        string symLink = hdf["SymLink"].getString();
        AccessLogs.
          push_back(AccessLogFileData(fname, symLink, hdf["Format"].
                                      getString(AccessLogDefaultFormat)));
      }
    }

    AdminLogFormat = logger["AdminLog.Format"].getString("%h %t %s %U");
    AdminLogFile = logger["AdminLog.File"].getString();
    AdminLogSymLink = logger["AdminLog.SymLink"].getString();
  }
  {
    Hdf error = config["ErrorHandling"];

    /* Remove this, once its removed from production configs */
    (void)error["NoInfiniteLoopDetection"].getBool();

    MaxSerializedStringSize =
      error["MaxSerializedStringSize"].getInt32(64 * 1024 * 1024);
    CallUserHandlerOnFatals = error["CallUserHandlerOnFatals"].getBool(true);
    ThrowExceptionOnBadMethodCall =
      error["ThrowExceptionOnBadMethodCall"].getBool(true);
    MaxLoopCount = error["MaxLoopCount"].getInt32(0);
    NoInfiniteRecursionDetection =
      error["NoInfiniteRecursionDetection"].getBool();
    ThrowBadTypeExceptions = error["ThrowBadTypeExceptions"].getBool();
    ThrowTooManyArguments = error["ThrowTooManyArguments"].getBool();
    WarnTooManyArguments = error["WarnTooManyArguments"].getBool();
    ThrowMissingArguments = error["ThrowMissingArguments"].getBool();
    ThrowInvalidArguments = error["ThrowInvalidArguments"].getBool();
    EnableHipHopErrors = error["EnableHipHopErrors"].getBool(true);
    AssertActive = error["AssertActive"].getBool();
    AssertWarning = error["AssertWarning"].getBool();
    NoticeFrequency = error["NoticeFrequency"].getInt64(1);
    WarningFrequency = error["WarningFrequency"].getInt64(1);
  }
  {
    Hdf rlimit = config["ResourceLimit"];
    if (rlimit["CoreFileSizeOverride"].getInt64()) {
      setResourceLimit(RLIMIT_CORE,   rlimit, "CoreFileSizeOverride");
    } else {
      setResourceLimit(RLIMIT_CORE,   rlimit, "CoreFileSize");
    }
    setResourceLimit(RLIMIT_NOFILE, rlimit, "MaxSocket");
    setResourceLimit(RLIMIT_DATA,   rlimit, "RSS");
    MaxRSS = rlimit["MaxRSS"].getInt64(0);
    SocketDefaultTimeout = rlimit["SocketDefaultTimeout"].getInt64(5);
    MaxRSSPollingCycle = rlimit["MaxRSSPollingCycle"].getInt64(0);
    DropCacheCycle = rlimit["DropCacheCycle"].getInt64(0);
    MaxSQLRowCount = rlimit["MaxSQLRowCount"].getInt64(0);
    MaxMemcacheKeyCount = rlimit["MaxMemcacheKeyCount"].getInt64(0);
    SerializationSizeLimit =
      rlimit["SerializationSizeLimit"].getInt64(StringData::MaxSize);
    StringOffsetLimit = rlimit["StringOffsetLimit"].getInt64(10 * 1024 * 1024);
  }
  {
    Hdf server = config["Server"];
    Host = server["Host"].getString();
    DefaultServerNameSuffix = server["DefaultServerNameSuffix"].getString();
    ServerType = server["Type"].getString(ServerType);
    IniSetting::Bind(IniSetting::CORE, IniSetting::PHP_INI_SYSTEM,
                     "hhvm.server.type", &ServerType);
    ServerIP = server["IP"].getString();
    ServerFileSocket = server["FileSocket"].getString();
    IniSetting::Bind(IniSetting::CORE, IniSetting::PHP_INI_SYSTEM,
                     "hhvm.server.file_socket", &ServerFileSocket);
    ServerPrimaryIP = GetPrimaryIP();
    ServerPort = server["Port"].getUInt16(80);
    IniSetting::Bind(IniSetting::CORE, IniSetting::PHP_INI_SYSTEM,
                     "hhvm.server.port", &ServerPort);
    ServerBacklog = server["Backlog"].getInt16(128);
    ServerConnectionLimit = server["ConnectionLimit"].getInt16(0);
    ServerThreadCount = server["ThreadCount"].getInt32(50);
    ServerThreadRoundRobin = server["ThreadRoundRobin"].getBool();
    ServerWarmupThrottleRequestCount =
      server["WarmupThrottleRequestCount"].getInt32(
        ServerWarmupThrottleRequestCount
      );
    ServerThreadDropCacheTimeoutSeconds =
      server["ThreadDropCacheTimeoutSeconds"].getInt32(0);
    if (server["ThreadJobLIFO"].getBool()) {
      ServerThreadJobLIFOSwitchThreshold = 0;
    }
    ServerThreadJobLIFOSwitchThreshold =
      server["ThreadJobLIFOSwitchThreshold"].getInt32(
        ServerThreadJobLIFOSwitchThreshold);
    ServerThreadJobMaxQueuingMilliSeconds =
      server["ThreadJobMaxQueuingMilliSeconds"].getInt16(-1);
    ServerThreadDropStack = server["ThreadDropStack"].getBool();
    ServerHttpSafeMode = server["HttpSafeMode"].getBool();
    ServerStatCache = server["StatCache"].getBool(false);
    server["WarmupRequests"].get(ServerWarmupRequests);
    server["HighPriorityEndPoints"].get(ServerHighPriorityEndPoints);
    ServerExitOnBindFail = server["ExitOnBindFail"].getBool(false);

    RequestTimeoutSeconds = server["RequestTimeoutSeconds"].getInt32(0);
    PspTimeoutSeconds = server["PspTimeoutSeconds"].getInt32(0);
    ServerMemoryHeadRoom = server["MemoryHeadRoom"].getInt64(0);
    RequestMemoryMaxBytes = server["RequestMemoryMaxBytes"].
      getInt64(std::numeric_limits<int64_t>::max());
    ResponseQueueCount = server["ResponseQueueCount"].getInt32(0);
    if (ResponseQueueCount <= 0) {
      ResponseQueueCount = ServerThreadCount / 10;
      if (ResponseQueueCount <= 0) ResponseQueueCount = 1;
    }
    ServerGracefulShutdownWait = server["GracefulShutdownWait"].getInt16(0);
    ServerHarshShutdown = server["HarshShutdown"].getBool(true);
    ServerEvilShutdown = server["EvilShutdown"].getBool(true);
    ServerDanglingWait = server["DanglingWait"].getInt16(0);
    ServerShutdownListenWait = server["ShutdownListenWait"].getInt16(0);
    ServerShutdownListenNoWork = server["ShutdownListenNoWork"].getInt16(-1);
    server["SSLNextProtocols"].get(ServerNextProtocols);
    if (ServerGracefulShutdownWait < ServerDanglingWait) {
      ServerGracefulShutdownWait = ServerDanglingWait;
    }
    GzipCompressionLevel = server["GzipCompressionLevel"].getInt16(3);

    ForceCompressionURL    = server["ForceCompression"]["URL"].getString();
    ForceCompressionCookie = server["ForceCompression"]["Cookie"].getString();
    ForceCompressionParam  = server["ForceCompression"]["Param"].getString();

    EnableMagicQuotesGpc = server["EnableMagicQuotesGpc"].getBool();
    EnableKeepAlive = server["EnableKeepAlive"].getBool(true);
    ExposeHPHP = server["ExposeHPHP"].getBool(true);
    ExposeXFBServer = server["ExposeXFBServer"].getBool(false);
    ExposeXFBDebug = server["ExposeXFBDebug"].getBool(false);
    XFBDebugSSLKey = server["XFBDebugSSLKey"].getString("");
    ConnectionTimeoutSeconds = server["ConnectionTimeoutSeconds"].getInt16(-1);
    EnableOutputBuffering = server["EnableOutputBuffering"].getBool();
    OutputHandler = server["OutputHandler"].getString();
    ImplicitFlush = server["ImplicitFlush"].getBool();
    EnableEarlyFlush = server["EnableEarlyFlush"].getBool(true);
    ForceChunkedEncoding = server["ForceChunkedEncoding"].getBool();
    MaxPostSize = (server["MaxPostSize"].getInt32(100)) * (1LL << 20);
    AlwaysPopulateRawPostData =
      server["AlwaysPopulateRawPostData"].getBool(true);
    LibEventSyncSend = server["LibEventSyncSend"].getBool(true);
    TakeoverFilename = server["TakeoverFilename"].getString();
    ExpiresActive = server["ExpiresActive"].getBool(true);
    ExpiresDefault = server["ExpiresDefault"].getInt32(2592000);
    if (ExpiresDefault < 0) ExpiresDefault = 2592000;
    DefaultCharsetName = server["DefaultCharsetName"].getString("utf-8");

    RequestBodyReadLimit = server["RequestBodyReadLimit"].getInt32(-1);

    EnableSSL = server["EnableSSL"].getBool();
    SSLPort = server["SSLPort"].getUInt16(443);
    SSLCertificateFile = server["SSLCertificateFile"].getString();
    SSLCertificateKeyFile = server["SSLCertificateKeyFile"].getString();
    SSLCertificateDir = server["SSLCertificateDir"].getString();
    TLSDisableTLS1_2 = server["TLSDisableTLS1_2"].getBool(false);
    TLSClientCipherSpec = server["TLSClientCipherSpec"].getString();

    string srcRoot = FileUtil::normalizeDir(server["SourceRoot"].getString());
    if (!srcRoot.empty()) SourceRoot = srcRoot;
    FileCache::SourceRoot = SourceRoot;

    server["IncludeSearchPaths"].get(IncludeSearchPaths);
    for (unsigned int i = 0; i < IncludeSearchPaths.size(); i++) {
      IncludeSearchPaths[i] = FileUtil::normalizeDir(IncludeSearchPaths[i]);
    }
    IncludeSearchPaths.insert(IncludeSearchPaths.begin(), ".");

    FileCache = server["FileCache"].getString();
    DefaultDocument = server["DefaultDocument"].getString();
    IniSetting::Bind(IniSetting::CORE, IniSetting::PHP_INI_SYSTEM,
                     "hhvm.server.default_document", &DefaultDocument);
    ErrorDocument404 = server["ErrorDocument404"].getString();
    normalizePath(ErrorDocument404);
    ForbiddenAs404 = server["ForbiddenAs404"].getBool();
    ErrorDocument500 = server["ErrorDocument500"].getString();
    normalizePath(ErrorDocument500);
    FatalErrorMessage = server["FatalErrorMessage"].getString();
    FontPath = FileUtil::normalizeDir(server["FontPath"].getString());
    EnableStaticContentFromDisk =
      server["EnableStaticContentFromDisk"].getBool(true);
    EnableOnDemandUncompress =
      server["EnableOnDemandUncompress"].getBool(true);
    EnableStaticContentMMap =
      server["EnableStaticContentMMap"].getBool(true);
    if (EnableStaticContentMMap) {
      EnableOnDemandUncompress = true;
    }
    Utf8izeReplace = server["Utf8izeReplace"].getBool(true);

    StartupDocument = server["StartupDocument"].getString();
    normalizePath(StartupDocument);
    WarmupDocument = server["WarmupDocument"].getString();
    RequestInitFunction = server["RequestInitFunction"].getString();
    RequestInitDocument = server["RequestInitDocument"].getString();
    server["ThreadDocuments"].get(ThreadDocuments);
    for (unsigned int i = 0; i < ThreadDocuments.size(); i++) {
      normalizePath(ThreadDocuments[i]);
    }
    server["ThreadLoopDocuments"].get(ThreadLoopDocuments);
    for (unsigned int i = 0; i < ThreadLoopDocuments.size(); i++) {
      normalizePath(ThreadLoopDocuments[i]);
    }

    SafeFileAccess = server["SafeFileAccess"].getBool();
    server["AllowedDirectories"].get(AllowedDirectories);

    WhitelistExec = server["WhitelistExec"].getBool();
    WhitelistExecWarningOnly = server["WhitelistExecWarningOnly"].getBool();
    server["AllowedExecCmds"].get(AllowedExecCmds);

    UnserializationWhitelistCheck =
      server["UnserializationWhitelistCheck"].getBool(false);
    UnserializationWhitelistCheckWarningOnly =
      server["UnserializationWhitelistCheckWarningOnly"].getBool(true);

    server["AllowedFiles"].get(AllowedFiles);

    server["ForbiddenFileExtensions"].get(ForbiddenFileExtensions);

    LockCodeMemory = server["LockCodeMemory"].getBool(false);
    MaxArrayChain = server["MaxArrayChain"].getInt32(INT_MAX);
    if (MaxArrayChain != INT_MAX) {
      // HphpArray needs a higher threshold to avoid false-positives.
      // (and we always use HphpArray)
      MaxArrayChain *= 2;
    }

    WarnOnCollectionToArray = server["WarnOnCollectionToArray"].getBool(false);
    UseDirectCopy = server["UseDirectCopy"].getBool(false);
    AlwaysUseRelativePath = server["AlwaysUseRelativePath"].getBool(false);

    Hdf dns = server["DnsCache"];
    EnableDnsCache = dns["Enable"].getBool();
    DnsCacheTTL = dns["TTL"].getInt32(600); // 10 minutes
    DnsCacheKeyMaturityThreshold = dns["KeyMaturityThreshold"].getInt32(20);
    DnsCacheMaximumCapacity = dns["MaximumCapacity"].getInt64(0);
    DnsCacheKeyFrequencyUpdatePeriod = dns["KeyFrequencyUpdatePeriod"].
      getInt32(1000);

    Hdf upload = server["Upload"];
    UploadMaxFileSize =
      (upload["UploadMaxFileSize"].getInt32(100)) * (1LL << 20);
    UploadTmpDir = upload["UploadTmpDir"].getString("/tmp");
    EnableFileUploads = upload["EnableFileUploads"].getBool(true);
    EnableUploadProgress = upload["EnableUploadProgress"].getBool();
    Rfc1867Freq = upload["Rfc1867Freq"].getInt32(256 * 1024);
    if (Rfc1867Freq < 0) Rfc1867Freq = 256 * 1024;
    Rfc1867Prefix = upload["Rfc1867Prefix"].getString("vupload_");
    Rfc1867Name = upload["Rfc1867Name"].getString("video_ptoken");

    ImageMemoryMaxBytes = server["ImageMemoryMaxBytes"].getInt64(0);
    if (ImageMemoryMaxBytes == 0) {
      ImageMemoryMaxBytes = UploadMaxFileSize * 2;
    }
    SharedStores::Create();

    LightProcessFilePrefix =
      server["LightProcessFilePrefix"].getString("./lightprocess");
    LightProcessCount = server["LightProcessCount"].getInt32(0);

    InjectedStackTrace = server["InjectedStackTrace"].getBool(true);
    InjectedStackTraceLimit = server["InjectedStackTraceLimit"].getInt32(-1);

    ForceServerNameToHeader = server["ForceServerNameToHeader"].getBool();

    EnableCufAsync = server["EnableCufAsync"].getBool(false);
    PathDebug = server["PathDebug"].getBool(false);

    ServerUser = server["User"].getString("");
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
    XboxServerThreadCount = xbox["ServerInfo.ThreadCount"].getInt32(10);
    XboxServerMaxQueueLength =
      xbox["ServerInfo.MaxQueueLength"].getInt32(INT_MAX);
    if (XboxServerMaxQueueLength < 0) XboxServerMaxQueueLength = INT_MAX;
    XboxServerPort = xbox["ServerInfo.Port"].getInt32(0);
    XboxDefaultLocalTimeoutMilliSeconds =
      xbox["DefaultLocalTimeoutMilliSeconds"].getInt32(500);
    XboxDefaultRemoteTimeoutSeconds =
      xbox["DefaultRemoteTimeoutSeconds"].getInt32(5);
    XboxServerInfoMaxRequest = xbox["ServerInfo.MaxRequest"].getInt32(500);
    XboxServerInfoDuration = xbox["ServerInfo.MaxDuration"].getInt32(120);
    XboxServerInfoWarmupDoc = xbox["ServerInfo.WarmupDocument"].get("");
    XboxServerInfoReqInitFunc = xbox["ServerInfo.RequestInitFunction"].get("");
    XboxServerInfoReqInitDoc = xbox["ServerInfo.RequestInitDocument"].get("");
    XboxServerInfoAlwaysReset = xbox["ServerInfo.AlwaysReset"].getBool(false);
    XboxServerLogInfo = xbox["ServerInfo.LogInfo"].getBool(false);
    XboxProcessMessageFunc =
      xbox["ProcessMessageFunc"].get("xbox_process_message");
  }
  {
    Hdf pagelet = config["PageletServer"];
    PageletServerThreadCount = pagelet["ThreadCount"].getInt32(0);
    PageletServerThreadRoundRobin = pagelet["ThreadRoundRobin"].getBool();
    PageletServerThreadDropStack = pagelet["ThreadDropStack"].getBool();
    PageletServerThreadDropCacheTimeoutSeconds =
      pagelet["ThreadDropCacheTimeoutSeconds"].getInt32(0);
    PageletServerQueueLimit = pagelet["QueueLimit"].getInt32(0);
  }
  {
    FiberCount = config["Fiber.ThreadCount"].getInt32(Process::GetCPUCount());
  }
  {
    Hdf content = config["StaticFile"];
    content["Extensions"].get(StaticFileExtensions);
    content["Generators"].get(StaticFileGenerators);

    Hdf matches = content["FilesMatch"];
    if (matches.exists()) {
      for (Hdf hdf = matches.firstChild(); hdf.exists(); hdf = hdf.next()) {
        FilesMatches.push_back(std::make_shared<FilesMatch>(hdf));
      }
    }
  }
  {
    Hdf phpfile = config["PhpFile"];
    phpfile["Extensions"].get(PhpFileExtensions);
  }
  {
    Hdf admin = config["AdminServer"];
    AdminServerPort = admin["Port"].getUInt16(0);
    AdminThreadCount = admin["ThreadCount"].getInt32(1);
    AdminPassword = admin["Password"].getString();
    admin["Passwords"].get(AdminPasswords);
  }
  {
    Hdf proxy = config["Proxy"];
    ProxyOrigin = proxy["Origin"].getString();
    ProxyRetry = proxy["Retry"].getInt16(3);
    UseServeURLs = proxy["ServeURLs"].getBool();
    proxy["ServeURLs"].get(ServeURLs);
    UseProxyURLs = proxy["ProxyURLs"].getBool();
    ProxyPercentage = proxy["Percentage"].getByte(0);
    proxy["ProxyURLs"].get(ProxyURLs);
    proxy["ProxyPatterns"].get(ProxyPatterns);
  }
  {
    Hdf http = config["Http"];
    HttpDefaultTimeout = http["DefaultTimeout"].getInt32(30);
    HttpSlowQueryThreshold = http["SlowQueryThreshold"].getInt32(5000);
  }
  {
    Hdf debug = config["Debug"];
    NativeStackTrace = debug["NativeStackTrace"].getBool();
    StackTrace::Enabled = NativeStackTrace;
    TranslateLeakStackTrace = debug["TranslateLeakStackTrace"].getBool();
    FullBacktrace = debug["FullBacktrace"].getBool();
    ServerStackTrace = debug["ServerStackTrace"].getBool();
    ServerErrorMessage = debug["ServerErrorMessage"].getBool();
    TranslateSource = debug["TranslateSource"].getBool();
    RecordInput = debug["RecordInput"].getBool();
    ClearInputOnSuccess = debug["ClearInputOnSuccess"].getBool(true);
    ProfilerOutputDir = debug["ProfilerOutputDir"].getString("/tmp");
    CoreDumpEmail = debug["CoreDumpEmail"].getString();
    CoreDumpReport = debug["CoreDumpReport"].getBool(true);
    if (CoreDumpReport) {
      install_crash_reporter();
    }
    CoreDumpReportDirectory =
      debug["CoreDumpReportDirectory"].getString(CoreDumpReportDirectory);
    LocalMemcache = debug["LocalMemcache"].getBool();
    MemcacheReadOnly = debug["MemcacheReadOnly"].getBool();

    {
      Hdf simpleCounter = debug["SimpleCounter"];
      SimpleCounter::SampleStackCount =
        simpleCounter["SampleStackCount"].getInt32(0);
      SimpleCounter::SampleStackDepth =
        simpleCounter["SampleStackDepth"].getInt32(5);
    }
  }
  {
    Hdf stats = config["Stats"];
    EnableStats = stats.getBool(); // main switch

    EnableWebStats = stats["Web"].getBool();
    EnableMemoryStats = stats["Memory"].getBool();
    EnableMemcacheStats = stats["Memcache"].getBool();
    EnableMemcacheKeyStats = stats["MemcacheKey"].getBool();
    EnableSQLStats = stats["SQL"].getBool();
    EnableSQLTableStats = stats["SQLTable"].getBool();
    EnableNetworkIOStatus = stats["NetworkIO"].getBool();

    StatsXSL = stats["XSL"].getString();
    StatsXSLProxy = stats["XSLProxy"].getString();

    StatsSlotDuration = stats["SlotDuration"].getInt32(10 * 60); // 10 minutes
    StatsMaxSlot = stats["MaxSlot"].getInt32(12 * 6); // 12 hours

    EnableHotProfiler = stats["EnableHotProfiler"].getBool(true);
    ProfilerTraceBuffer = stats["ProfilerTraceBuffer"].getInt32(2000000);
    ProfilerTraceExpansion = stats["ProfilerTraceExpansion"].getDouble(1.2);
    ProfilerMaxTraceBuffer = stats["ProfilerMaxTraceBuffer"].getInt32(0);
  }
  {
    config["ServerVariables"].get(ServerVariables);
    config["EnvVariables"].get(EnvVariables);
  }
  {
    Hdf eval = config["Eval"];
    EnableHipHopSyntax = eval["EnableHipHopSyntax"].getBool();
    EnableHipHopExperimentalSyntax =
      eval["EnableHipHopExperimentalSyntax"].getBool();
    EnableShortTags= eval["EnableShortTags"].getBool(true);
    EnableAspTags = eval["EnableAspTags"].getBool();
    EnableXHP = eval["EnableXHP"].getBool(false);
    EnableZendCompat = eval["EnableZendCompat"].getBool(false);
    TimeoutsUseWallTime = eval["TimeoutsUseWallTime"].getBool(true);
    CheckFlushOnUserClose = eval["CheckFlushOnUserClose"].getBool(true);

    if (EnableHipHopSyntax) {
      // If EnableHipHopSyntax is true, it forces EnableXHP to true
      // regardless of how it was set in the config
      EnableXHP = true;
    }

    EnableObjDestructCall = eval["EnableObjDestructCall"].getBool(false);
    MaxUserFunctionId = eval["MaxUserFunctionId"].getInt32(2 * 65536);
    CheckSymLink = eval["CheckSymLink"].getBool(true);

    EnableAlternative = eval["EnableAlternative"].getInt32(0);

#define get_double getDouble
#define get_bool getBool
#define get_string getString
#define get_int16 getInt16
#define get_int32 getInt32
#define get_int32_t getInt32
#define get_int64 getInt64
#define get_uint16 getUInt16
#define get_uint32 getUInt32
#define get_uint32_t getUInt32
#define get_uint64 getUInt64
#define get_uint64_t getUInt64
#define F(type, name, defaultVal) \
    Eval ## name = eval[#name].get_ ##type(defaultVal);
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
    EnableEmitSwitch = eval["EnableEmitSwitch"].getBool(true);
    EnableEmitterStats = eval["EnableEmitterStats"].getBool(EnableEmitterStats);
    EnableInstructionCounts = eval["EnableInstructionCounts"].getBool(false);
    RecordCodeCoverage = eval["RecordCodeCoverage"].getBool();
    if (EvalJit && RecordCodeCoverage) {
      throw InvalidArgumentException(
        "code coverage", "Code coverage is not supported for Eval.Jit=true");
    }
    if (RecordCodeCoverage) CheckSymLink = true;
    CodeCoverageOutputFile = eval["CodeCoverageOutputFile"].getString();
    {
      Hdf debugger = eval["Debugger"];
      EnableDebugger = debugger["EnableDebugger"].getBool();
      EnableDebuggerColor = debugger["EnableDebuggerColor"].getBool(true);
      EnableDebuggerPrompt = debugger["EnableDebuggerPrompt"].getBool(true);
      EnableDebuggerServer = debugger["EnableDebuggerServer"].getBool();
      EnableDebuggerUsageLog = debugger["EnableDebuggerUsageLog"].getBool();
      DebuggerServerPort = debugger["Port"].getUInt16(8089);
      DebuggerDisableIPv6 = debugger["DisableIPv6"].getBool(false);
      DebuggerDefaultSandboxPath = debugger["DefaultSandboxPath"].getString();
      DebuggerStartupDocument = debugger["StartupDocument"].getString();
      DebuggerSignalTimeout = debugger["SignalTimeout"].getInt32(1);

      DebuggerDefaultRpcPort = debugger["RPC.DefaultPort"].getUInt16(8083);
      DebuggerDefaultRpcAuth = debugger["RPC.DefaultAuth"].getString();
      DebuggerRpcHostDomain = debugger["RPC.HostDomain"].getString();
      DebuggerDefaultRpcTimeout = debugger["RPC.DefaultTimeout"].getInt32(30);
    }
    {
      Hdf lang = config["Hack"]["Lang"];
      IntsOverflowToInts =
        lang["IntsOverflowToInts"].getBool(EnableHipHopSyntax);
    }
    {
      Hdf repo = config["Repo"];
      {
        Hdf repoLocal = repo["Local"];
        // Repo.Local.Mode.
        RepoLocalMode = repoLocal["Mode"].getString();
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
        RepoLocalPath = repoLocal["Path"].getString();
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
        RepoCentralPath = repoCentral["Path"].getString();
        IniSetting::Bind(IniSetting::CORE, IniSetting::PHP_INI_SYSTEM,
                         "hhvm.repo.central.path", &RepoCentralPath);
      }
      {
        Hdf repoEval = repo["Eval"];
        // Repo.Eval.Mode.
        RepoEvalMode = repoEval["Mode"].getString();
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
      RepoJournal = repo["Journal"].getString("delete");
      RepoCommit = repo["Commit"].getBool(true);
      RepoDebugInfo = repo["DebugInfo"].getBool(true);
      RepoAuthoritative = repo["Authoritative"].getBool(false);
    }

    // NB: after we know the value of RepoAuthoritative.
    EnableArgsInBacktraces =
      eval["EnableArgsInBacktraces"].getBool(!RepoAuthoritative);
    EvalAuthoritativeMode =
      eval["AuthoritativeMode"].getBool(false) || RepoAuthoritative;
  }
  {
    Hdf sandbox = config["Sandbox"];
    SandboxMode = sandbox["SandboxMode"].getBool();
    SandboxPattern = format_pattern(sandbox["Pattern"].getString(), true);
    SandboxHome = sandbox["Home"].getString();
    SandboxFallback = sandbox["Fallback"].getString();
    SandboxConfFile = sandbox["ConfFile"].getString();
    SandboxFromCommonRoot = sandbox["FromCommonRoot"].getBool();
    SandboxDirectoriesRoot = sandbox["DirectoriesRoot"].getString();
    SandboxLogsRoot = sandbox["LogsRoot"].getString();
    sandbox["ServerVariables"].get(SandboxServerVariables);
  }
  {
    Hdf mail = config["Mail"];
    SendmailPath = mail["SendmailPath"].getString("sendmail -t -i");
    MailForceExtraParameters = mail["ForceExtraParameters"].getString();
  }
  {
    Hdf preg = config["Preg"];
    PregBacktraceLimit = preg["BacktraceLimit"].getInt64(1000000);
    PregRecursionLimit = preg["RecursionLimit"].getInt64(100000);
    EnablePregErrorLog = preg["ErrorLog"].getBool(true);
  }
  {
    Hdf hhprofServer = config["HHProfServer"];
    HHProfServerEnabled = hhprofServer["Enabled"].getBool(false);
    HHProfServerPort = hhprofServer["Port"].getUInt16(4327);
    HHProfServerThreads = hhprofServer["Threads"].getInt16(2);
    HHProfServerTimeoutSeconds =
      hhprofServer["TimeoutSeconds"].getInt64(30);
    HHProfServerProfileClientMode =
      hhprofServer["ProfileClientMode"].getBool(true);
    HHProfServerAllocationProfile =
      hhprofServer["AllocationProfile"].getBool(false);

    // HHProfServer.Filter.*
    Hdf hhprofFilter = hhprofServer["Filter"];
    HHProfServerFilterMinAllocPerReq =
      hhprofFilter["MinAllocPerReq"].getInt64(2);
    HHProfServerFilterMinBytesPerReq =
      hhprofFilter["MinBytesPerReq"].getInt64(128);
  }
  {
    Hdf simplexml = config["SimpleXML"];
    SimpleXMLEmptyNamespaceMatchesAll =
      simplexml["EmptyNamespaceMatchesAll"].getBool(false);
  }
#ifdef FACEBOOK
  {
    Hdf fb303Server = config["Fb303Server"];
    EnableFb303Server = fb303Server["Enable"].getBool(true);
    Fb303ServerPort = fb303Server["Port"].getUInt16(0);
    Fb303ServerThreadStackSizeMb = fb303Server["ThreadStackSizeMb"].getInt16(8);
    Fb303ServerWorkerThreads = fb303Server["WorkerThreads"].getInt16(1);
    Fb303ServerPoolThreads = fb303Server["PoolThreads"].getInt16(1);
  }
#endif

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
  if (overwrites) Loaded = true;
}

///////////////////////////////////////////////////////////////////////////////
}
