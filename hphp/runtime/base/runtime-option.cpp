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

#include "folly/String.h"

#include "hphp/util/hdf.h"
#include "hphp/util/util.h"
#include "hphp/util/network.h"
#include "hphp/util/logger.h"
#include "hphp/util/stack-trace.h"
#include "hphp/util/process.h"
#include "hphp/util/file-cache.h"
#include "hphp/util/log-file-flusher.h"

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
#include "hphp/runtime/base/ini-setting.h"

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
bool RuntimeOption::EnableApplicationLog = true;
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

int RuntimeOption::GetScannerType() {
  int type = 0;
  if (EnableShortTags) type |= Scanner::AllowShortTags;
  if (EnableAspTags) type |= Scanner::AllowAspTags;
  if (EnableXHP) type |= Scanner::AllowXHPSyntax;
  if (EnableHipHopSyntax) type |= Scanner::AllowHipHopSyntax;
  return type;
}

// Initializers for Eval flags.
static inline bool evalJitDefault() {
  // --mode server or --mode daemon
  // run long enough to justify JIT
  if (RuntimeOption::ServerExecutionMode()) {
    return true;
  }

  // JIT explicitly turned on via .hhvm-jit file
  static const char* path = "/.hhvm-jit";
  struct stat dummy;
  return stat(path, &dummy) == 0;
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
static const int kDefaultJitPGOThreshold = debug ? 2 : 4;
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

// TODO (t3610856): Change the default to false once dependent code is fixed
bool RuntimeOption::SimpleXMLEmptyNamespaceMatchesAll = true;

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
                         const IniSetting::Map &ini,
                         std::vector<std::string> *overwrites /* = NULL */,
                         bool empty /* = false */) {
  // Machine metrics
  std::string hostname, tier, cpu;
  {
    Hdf machine = config["Machine"];

    hostname = GetString(ini, machine, "name", "");
    if (hostname.empty()) {
      hostname = Process::GetHostName();
    }
    tier = GetString(ini, machine, "tier", "");
    cpu = GetString(ini, machine, "cpu", "");
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

  Bind(PidFile, ini, config, "PidFile");

  Bind(DynamicInvokeFunctions, ini, config, "DynamicInvokeFunctions");

  {
    Hdf logger = config["Log"];
    Bind(ini, logger, "Level",
      [](const std::string &value, void* p) {
        auto &level = *(Logger::LogLevelType*)p;
        if (value == "None") {
          level = Logger::LogNone;
        } else if (value == "Error") {
          level = Logger::LogError;
        } else if (value == "Warning") {
          level = Logger::LogWarning;
        } else if (value == "Info") {
          level = Logger::LogInfo;
        } else if (value == "Verbose") {
          level = Logger::LogVerbose;
        }
        return true;
      },
      ini_get_int,
      &Logger::LogLevel
    );
    Bind(Logger::LogHeader, ini, logger, "Header");
    auto logInjectedStackTrace =
      GetBool(ini, logger, "InjectedStackTrace", false);
    if (logInjectedStackTrace) {
      Logger::SetTheLogger(new ExtendedLogger());
      ExtendedLogger::EnabledByDefault = true;
    }
    Bind(Logger::LogNativeStackTrace, ini, logger, "NativeStackTrace");
    Bind(Logger::MaxMessagesPerRequest, ini, logger, "MaxMessagesPerRequest");
    Bind(Logger::UseSyslog, ini, logger, "UseSyslog");
    Bind(Logger::UseLogFile, ini, logger, "UseLogFile");
    Bind(Logger::UseCronolog, ini, logger, "UseCronolog");
    if (Logger::UseLogFile) {
      Bind(LogFile, ini, logger, "File");
      if (!RuntimeOption::ServerExecutionMode()) {
        LogFile.clear();
      }
      if (LogFile[0] == '|') Logger::IsPipeOutput = true;
      Bind(LogFileSymLink, ini, logger, "SymLink");
    }
    Bind(LogFileFlusher::DropCacheChunkSize, ini, logger, "DropCacheChunkSize");
    Bind(AlwaysEscapeLog, ini, logger, "AlwaysEscapeLog");
    Bind(RuntimeOption::LogHeaderMangle, ini, logger, "HeaderMangle");

    Bind(AlwaysLogUnhandledExceptions, ini, logger,
        "AlwaysLogUnhandledExceptions");
    Bind(NoSilencer, ini, logger, "NoSilencer");
    Bind(EnableApplicationLog, ini, logger, "ApplicationLog");
    Bind(RuntimeErrorReportingLevel, ini, logger, "RuntimeErrorReportingLevel");

    Bind(AccessLogDefaultFormat, ini, logger, "AccessLogDefaultFormat");
    {
      Hdf access = logger["Access"];
      for (Hdf hdf = access.firstChild(); hdf.exists();
           hdf = hdf.next()) {
        std::string fname;
        Bind(fname, ini, hdf, "File");
        if (fname.empty()) {
          continue;
        }
        std::string symLink;
        Bind(symLink, ini, hdf, "SymLink");
        std::string format;
        Bind(format, ini, hdf, "Format");
        AccessLogs.push_back(AccessLogFileData(
          fname,
          symLink,
          !format.empty() ? format : AccessLogDefaultFormat
        ));
      }
    }

    Bind(AdminLogFormat, ini, logger, "AdminLog.Format");
    Bind(AdminLogFile, ini, logger, "AdminLog.File");
    Bind(AdminLogSymLink, ini, logger, "AdminLog.SymLink");
  }
  {
    Hdf error = config["ErrorHandling"];

    /* Remove this, once its removed from production configs */
    (void)error["NoInfiniteLoopDetection"].setVisited();

    Bind(MaxSerializedStringSize, ini, error, "MaxSerializedStringSize");
    Bind(CallUserHandlerOnFatals, ini, error, "CallUserHandlerOnFatals");
    Bind(ThrowExceptionOnBadMethodCall, ini, error,
        "ThrowExceptionOnBadMethodCall");
    Bind(MaxLoopCount, ini, error, "MaxLoopCount");
    Bind(NoInfiniteRecursionDetection, ini, error,
        "NoInfiniteRecursionDetection");
    Bind(ThrowBadTypeExceptions, ini, error, "ThrowBadTypeExceptions");
    Bind(ThrowTooManyArguments, ini, error, "ThrowTooManyArguments");
    Bind(WarnTooManyArguments, ini, error, "WarnTooManyArguments");
    Bind(ThrowMissingArguments, ini, error, "ThrowMissingArguments");
    Bind(ThrowInvalidArguments, ini, error, "ThrowInvalidArguments");
    Bind(EnableHipHopErrors, ini, error, "EnableHipHopErrors");
    Bind(AssertActive, ini, error, "AssertActive");
    Bind(AssertWarning, ini, error, "AssertWarning");
    Bind(NoticeFrequency, ini, error, "NoticeFrequency");
    Bind(WarningFrequency, ini, error, "WarningFrequency");
  }
  {
    Hdf rlimit = config["ResourceLimit"];
    int64_t CoreFileSizeOverride =
      GetInt(ini, rlimit, "CoreFileSizeOverride", 0);
    if (CoreFileSizeOverride) {
      SetResourceLimit(RLIMIT_CORE, ini, rlimit, "CoreFileSizeOverride");
    } else {
      SetResourceLimit(RLIMIT_CORE, ini, rlimit, "CoreFileSize");
    }
    SetResourceLimit(RLIMIT_NOFILE, ini, rlimit, "MaxSocket");
    SetResourceLimit(RLIMIT_DATA, ini, rlimit, "RSS");
    Bind(MaxRSS, ini, rlimit, "MaxRSS");
    Bind(SocketDefaultTimeout, ini, rlimit, "SocketDefaultTimeout");
    Bind(MaxRSSPollingCycle, ini, rlimit, "MaxRSSPollingCycle");
    Bind(DropCacheCycle, ini, rlimit, "DropCacheCycle");
    Bind(MaxSQLRowCount, ini, rlimit, "MaxSQLRowCount");
    Bind(MaxMemcacheKeyCount, ini, rlimit, "MaxMemcacheKeyCount");
    Bind(SerializationSizeLimit, ini, rlimit, "SerializationSizeLimit");
    Bind(StringOffsetLimit, ini, rlimit, "StringOffsetLimit");
  }
  {
    Hdf server = config["Server"];
    Bind(Host, ini, server, "Host");
    Bind(DefaultServerNameSuffix, ini, server, "DefaultServerNameSuffix");
    Bind(ServerType, ini, server, "Type");
    Bind(ServerIP, ini, server, "IP");
    Bind(ServerFileSocket, ini, server, "FileSocket");
    ServerPrimaryIP = Util::GetPrimaryIP();
    Bind(ServerPort, ini, server, "Port");
    Bind(ServerBacklog, ini, server, "Backlog");
    Bind(ServerConnectionLimit, ini, server, "ConnectionLimit");
    Bind(ServerThreadCount, ini, server, "ThreadCount");
    Bind(ServerThreadRoundRobin, ini, server, "ThreadRoundRobin");
    Bind(ServerWarmupThrottleRequestCount, ini, server,
        "WarmupThrottleRequestCount");
    Bind(ServerThreadDropCacheTimeoutSeconds, ini, server,
        "ThreadDropCacheTimeoutSeconds");
    bool ThreadJobLIFO = GetBool(ini, server, "ThreadJobLIFO", false);
    if (ThreadJobLIFO) {
      ServerThreadJobLIFOSwitchThreshold = 0;
    }
    Bind(ServerThreadJobLIFOSwitchThreshold, ini, server,
        "ThreadJobLIFOSwitchThreshold");
    Bind(ServerThreadJobMaxQueuingMilliSeconds, ini, server,
        "ThreadJobMaxQueuingMilliSeconds");
    Bind(ServerThreadDropStack, ini, server, "ThreadDropStack");
    Bind(ServerHttpSafeMode, ini, server, "HttpSafeMode");
    Bind(ServerStatCache, ini, server, "StatCache");
    Bind(ServerWarmupRequests, ini, server, "WarmupRequests");
    Bind(ServerHighPriorityEndPoints, ini, server, "HighPriorityEndPoints");

    Bind(RequestTimeoutSeconds, ini, server, "RequestTimeoutSeconds");
    Bind(PspTimeoutSeconds, ini, server, "PspTimeoutSeconds");
    Bind(ServerMemoryHeadRoom, ini, server, "MemoryHeadRoom");
    Bind(RequestMemoryMaxBytes, ini, server, "RequestMemoryMaxBytes");
    Bind(ResponseQueueCount, ini, server, "ResponseQueueCount");
    if (ResponseQueueCount <= 0) {
      ResponseQueueCount = ServerThreadCount / 10;
      if (ResponseQueueCount <= 0) ResponseQueueCount = 1;
    }
    Bind(ServerGracefulShutdownWait, ini, server, "GracefulShutdownWait");
    Bind(ServerHarshShutdown, ini, server, "HarshShutdown");
    Bind(ServerEvilShutdown, ini, server, "EvilShutdown");
    Bind(ServerDanglingWait, ini, server, "DanglingWait");
    Bind(ServerShutdownListenWait, ini, server, "ShutdownListenWait");
    Bind(ServerShutdownListenNoWork, ini, server, "ShutdownListenNoWork");
    if (ServerGracefulShutdownWait < ServerDanglingWait) {
      ServerGracefulShutdownWait = ServerDanglingWait;
    }
    Bind(GzipCompressionLevel, ini, server, "GzipCompressionLevel");

    Bind(ForceCompressionURL, ini, server["ForceCompression"], "URL");
    Bind(ForceCompressionCookie, ini, server["ForceCompression"], "Cookie");
    Bind(ForceCompressionParam, ini, server["ForceCompression"], "Param");

    Bind(EnableMagicQuotesGpc, ini, server, "EnableMagicQuotesGpc");
    Bind(EnableKeepAlive, ini, server, "EnableKeepAlive");
    Bind(ExposeHPHP, ini, server, "ExposeHPHP");
    Bind(ExposeXFBServer, ini, server, "ExposeXFBServer");
    Bind(ExposeXFBDebug, ini, server, "ExposeXFBDebug");
    Bind(XFBDebugSSLKey, ini, server, "XFBDebugSSLKey");
    Bind(ConnectionTimeoutSeconds, ini, server, "ConnectionTimeoutSeconds");
    Bind(EnableOutputBuffering, ini, server, "EnableOutputBuffering");
    Bind(OutputHandler, ini, server, "OutputHandler");
    Bind(ImplicitFlush, ini, server, "ImplicitFlush");
    Bind(EnableEarlyFlush, ini, server, "EnableEarlyFlush");
    Bind(ForceChunkedEncoding, ini, server, "ForceChunkedEncoding");
    Bind(MaxPostSize, ini, server, "MaxPostSize");
    MaxPostSize *= 1LL << 20;
    Bind(AlwaysPopulateRawPostData, ini, server, "AlwaysPopulateRawPostData");
    Bind(LibEventSyncSend, ini, server, "LibEventSyncSend");
    Bind(TakeoverFilename, ini, server, "TakeoverFilename");
    Bind(ExpiresActive, ini, server, "ExpiresActive");
    Bind(ExpiresDefault, ini, server, "ExpiresDefault");
    if (ExpiresDefault < 0) ExpiresDefault = 2592000;
    Bind(DefaultCharsetName, ini, server, "DefaultCharsetName");

    Bind(RequestBodyReadLimit, ini, server, "RequestBodyReadLimit");

    Bind(EnableSSL, ini, server, "EnableSSL");
    Bind(SSLPort, ini, server, "SSLPort");
    Bind(SSLCertificateFile, ini, server, "SSLCertificateFile");
    Bind(SSLCertificateKeyFile, ini, server, "SSLCertificateKeyFile");
    Bind(SSLCertificateDir, ini, server, "SSLCertificateDir");
    Bind(TLSDisableTLS1_2, ini, server, "TLSDisableTLS1_2");
    Bind(TLSClientCipherSpec, ini, server, "TLSClientCipherSpec");

    Bind(ini, server, "SourceRoot",
      [](const std::string &value, void* p) {
        auto srcRoot = Util::normalizeDir(value);
        if (!srcRoot.empty()) {
          *(std::string*)p = srcRoot;
          FileCache::SourceRoot = srcRoot;
        }
        return true;
      },
      ini_get_stdstring,
      &SourceRoot
    );

    Bind(IncludeSearchPaths, ini, server, "IncludeSearchPaths");
    for (unsigned int i = 0; i < IncludeSearchPaths.size(); i++) {
      IncludeSearchPaths[i] = Util::normalizeDir(IncludeSearchPaths[i]);
    }
    IncludeSearchPaths.insert(IncludeSearchPaths.begin(), ".");

    Bind(FileCache, ini, server, "FileCache");
    Bind(DefaultDocument, ini, server, "DefaultDocument");
    Bind(ErrorDocument404, ini, server, "ErrorDocument404");
    normalizePath(ErrorDocument404);
    Bind(ForbiddenAs404, ini, server, "ForbiddenAs404");
    Bind(ErrorDocument500, ini, server, "ErrorDocument500");
    normalizePath(ErrorDocument500);
    Bind(FatalErrorMessage, ini, server, "FatalErrorMessage");
    Bind(FontPath, ini, server, "FontPath");
    FontPath = Util::normalizeDir(FontPath);
    Bind(EnableStaticContentFromDisk, ini, server,
        "EnableStaticContentFromDisk");
    Bind(EnableOnDemandUncompress, ini, server, "EnableOnDemandUncompress");
    Bind(EnableStaticContentMMap, ini, server, "EnableStaticContentMMap");
    if (EnableStaticContentMMap) {
      EnableOnDemandUncompress = true;
    }
    Bind(Utf8izeReplace, ini, server, "Utf8izeReplace");

    Bind(StartupDocument, ini, server, "StartupDocument");
    normalizePath(StartupDocument);
    Bind(WarmupDocument, ini, server, "WarmupDocument");
    Bind(RequestInitFunction, ini, server, "RequestInitFunction");
    Bind(RequestInitDocument, ini, server, "RequestInitDocument");
    Bind(ThreadDocuments, ini, server, "ThreadDocuments");
    for (unsigned int i = 0; i < ThreadDocuments.size(); i++) {
      normalizePath(ThreadDocuments[i]);
    }
    Bind(ThreadLoopDocuments, ini, server, "ThreadLoopDocuments");
    for (unsigned int i = 0; i < ThreadLoopDocuments.size(); i++) {
      normalizePath(ThreadLoopDocuments[i]);
    }

    Bind(SafeFileAccess, ini, server, "SafeFileAccess");
    Bind(AllowedDirectories, ini, server, "AllowedDirectories");

    Bind(WhitelistExec, ini, server, "WhitelistExec");
    Bind(WhitelistExecWarningOnly, ini, server, "WhitelistExecWarningOnly");
    Bind(AllowedExecCmds, ini, server, "AllowedExecCmds");

    Bind(UnserializationWhitelistCheck, ini, server,
        "UnserializationWhitelistCheck");
    Bind(UnserializationWhitelistCheckWarningOnly, ini, server,
        "UnserializationWhitelistCheckWarningOnly");

    Bind(AllowedFiles, ini, server, "AllowedFiles");

    Bind(ForbiddenFileExtensions, ini, server, "ForbiddenFileExtensions");

    Bind(LockCodeMemory, ini, server, "LockCodeMemory");
    Bind(MaxArrayChain, ini, server, "MaxArrayChain");
    if (MaxArrayChain != INT_MAX) {
      // HphpArray needs a higher threshold to avoid false-positives.
      // (and we always use HphpArray)
      MaxArrayChain *= 2;
    }

    Bind(WarnOnCollectionToArray, ini, server, "WarnOnCollectionToArray");
    Bind(UseDirectCopy, ini, server, "UseDirectCopy");
    Bind(AlwaysUseRelativePath, ini, server, "AlwaysUseRelativePath");

    Hdf dns = server["DnsCache"];
    Bind(EnableDnsCache, ini, dns, "Enable");
    Bind(DnsCacheTTL, ini, dns, "TTL"); // 10 minutes
    Bind(DnsCacheKeyMaturityThreshold, ini, dns, "KeyMaturityThreshold");
    Bind(DnsCacheMaximumCapacity, ini, dns, "MaximumCapacity");
    Bind(DnsCacheKeyFrequencyUpdatePeriod, ini, dns,
        "KeyFrequencyUpdatePeriod");

    Hdf upload = server["Upload"];
    Bind(UploadMaxFileSize, ini, upload, "UploadMaxFileSize");
    UploadMaxFileSize *= 1LL << 20;
    Bind(UploadTmpDir, ini, upload, "UploadTmpDir");
    RuntimeOption::AllowedDirectories.push_back(UploadTmpDir);
    Bind(EnableFileUploads, ini, upload, "EnableFileUploads");
    Bind(EnableUploadProgress, ini, upload, "EnableUploadProgress");
    Bind(Rfc1867Freq, ini, upload, "Rfc1867Freq");
    if (Rfc1867Freq < 0) Rfc1867Freq = 256 * 1024;
    Bind(Rfc1867Prefix, ini, upload, "Rfc1867Prefix");
    Bind(Rfc1867Name, ini, upload, "Rfc1867Name");

    Bind(ImageMemoryMaxBytes, ini, server, "ImageMemoryMaxBytes");
    if (ImageMemoryMaxBytes == 0) {
      ImageMemoryMaxBytes = UploadMaxFileSize * 2;
    }
    SharedStores::Create();

    Bind(LightProcessFilePrefix, ini, server, "LightProcessFilePrefix");
    Bind(LightProcessCount, ini, server, "LightProcessCount");

    Bind(InjectedStackTrace, ini, server, "InjectedStackTrace");
    Bind(InjectedStackTraceLimit, ini, server, "InjectedStackTraceLimit");

    Bind(ForceServerNameToHeader, ini, server, "ForceServerNameToHeader");

    Bind(EnableCufAsync, ini, server, "EnableCufAsync");
    Bind(PathDebug, ini, server, "PathDebug");

    Bind(ServerUser, ini, server, "User");
  }

  // Make sure at least the default virtual host is always initialized
  (void)VirtualHost::GetDefault();
  VirtualHost::SortAllowedDirectories(AllowedDirectories);
  {
    Hdf hosts = config["VirtualHost"];
    if (hosts.exists()) {
      for (Hdf hdf = hosts.firstChild(); hdf.exists(); hdf = hdf.next()) {
        if (hdf.getName() == "default") {
          VirtualHost::GetDefault().init(hdf, ini);
          VirtualHost::GetDefault().addAllowedDirectories(AllowedDirectories);
        } else {
          auto host = std::make_shared<VirtualHost>(hdf, ini);
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
    IpBlocks = std::make_shared<IpBlockMap>(ipblocks, ini);
  }
  {
    Hdf satellites = config["Satellites"];
    if (satellites.exists()) {
      for (Hdf hdf = satellites.firstChild(); hdf.exists(); hdf = hdf.next()) {
        auto satellite = std::make_shared<SatelliteServerInfo>(hdf, ini);
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
    Bind(XboxServerThreadCount, ini, xbox, "ServerInfo.ThreadCount");
    Bind(XboxServerMaxQueueLength, ini, xbox, "ServerInfo.MaxQueueLength");
    if (XboxServerMaxQueueLength < 0) XboxServerMaxQueueLength = INT_MAX;
    Bind(XboxServerPort, ini, xbox, "ServerInfo.Port");
    Bind(XboxDefaultLocalTimeoutMilliSeconds, ini, xbox,
        "DefaultLocalTimeoutMilliSeconds");
    Bind(XboxDefaultRemoteTimeoutSeconds, ini, xbox,
        "DefaultRemoteTimeoutSeconds");
    Bind(XboxServerInfoMaxRequest, ini, xbox, "ServerInfo.MaxRequest");
    Bind(XboxServerInfoDuration, ini, xbox, "ServerInfo.MaxDuration");
    Bind(XboxServerInfoWarmupDoc, ini, xbox, "ServerInfo.WarmupDocument");
    Bind(XboxServerInfoReqInitFunc, ini, xbox,
        "ServerInfo.RequestInitFunction");
    Bind(XboxServerInfoReqInitDoc, ini, xbox, "ServerInfo.RequestInitDocument");
    Bind(XboxServerInfoAlwaysReset, ini, xbox, "ServerInfo.AlwaysReset");
    Bind(XboxServerLogInfo, ini, xbox, "ServerInfo.LogInfo");
    Bind(XboxProcessMessageFunc, ini, xbox, "ProcessMessageFunc");
  }
  {
    Hdf pagelet = config["PageletServer"];
    Bind(PageletServerThreadCount, ini, pagelet, "ThreadCount");
    Bind(PageletServerThreadRoundRobin, ini, pagelet, "ThreadRoundRobin");
    Bind(PageletServerThreadDropStack, ini, pagelet, "ThreadDropStack");
    Bind(PageletServerThreadDropCacheTimeoutSeconds, ini, pagelet,
        "ThreadDropCacheTimeoutSeconds");
    Bind(PageletServerQueueLimit, ini, pagelet, "QueueLimit");
  }
  {
    Bind(FiberCount, ini, config, "Fiber.ThreadCount");
  }
  {
    Hdf content = config["StaticFile"];
    Bind(StaticFileExtensions, ini, content, "Extensions");
    Bind(StaticFileGenerators, ini, content, "Generators");

    Hdf matches = content["FilesMatch"];
    if (matches.exists()) {
      for (Hdf hdf = matches.firstChild(); hdf.exists(); hdf = hdf.next()) {
        FilesMatches.push_back(std::make_shared<FilesMatch>(hdf, ini));
      }
    }
  }
  {
    Hdf phpfile = config["PhpFile"];
    Bind(PhpFileExtensions, ini, phpfile, "Extensions");
  }
  {
    Hdf admin = config["AdminServer"];
    Bind(AdminServerPort, ini, admin, "Port");
    Bind(AdminThreadCount, ini, admin, "ThreadCount");
    Bind(AdminPassword, ini, admin, "Password");
    Bind(AdminPasswords, ini, admin, "Passwords");
  }
  {
    Hdf proxy = config["Proxy"];
    Bind(ProxyOrigin, ini, proxy, "Origin");
    Bind(ProxyRetry, ini, proxy, "Retry");
    Bind(UseServeURLs, ini, proxy, "ServeURLs");
    Bind(ServeURLs, ini, proxy, "ServeURLs");
    Bind(UseProxyURLs, ini, proxy, "ProxyURLs");
    Bind(ProxyPercentage, ini, proxy, "Percentage");
    Bind(ProxyURLs, ini, proxy, "ProxyURLs");
    Bind(ProxyPatterns, ini, proxy, "ProxyPatterns");
  }
  {
    Hdf http = config["Http"];
    Bind(HttpDefaultTimeout, ini, http, "DefaultTimeout");
    Bind(HttpSlowQueryThreshold, ini, http, "SlowQueryThreshold");
  }
  {
    Hdf debug = config["Debug"];
    Bind(NativeStackTrace, ini, debug, "NativeStackTrace");
    StackTrace::Enabled = NativeStackTrace;
    Bind(TranslateLeakStackTrace, ini, debug, "TranslateLeakStackTrace");
    Bind(FullBacktrace, ini, debug, "FullBacktrace");
    Bind(ServerStackTrace, ini, debug, "ServerStackTrace");
    Bind(ServerErrorMessage, ini, debug, "ServerErrorMessage");
    Bind(TranslateSource, ini, debug, "TranslateSource");
    Bind(RecordInput, ini, debug, "RecordInput");
    Bind(ClearInputOnSuccess, ini, debug, "ClearInputOnSuccess");
    Bind(ProfilerOutputDir, ini, debug, "ProfilerOutputDir");
    Bind(CoreDumpEmail, ini, debug, "CoreDumpEmail");
    Bind(CoreDumpReport, ini, debug, "CoreDumpReport");
    if (CoreDumpReport) {
      install_crash_reporter();
    }
    Bind(CoreDumpReportDirectory, ini, debug, "CoreDumpReportDirectory");
    Bind(LocalMemcache, ini, debug, "LocalMemcache");
    Bind(MemcacheReadOnly, ini, debug, "MemcacheReadOnly");

    {
      Hdf simpleCounter = debug["SimpleCounter"];
      Bind(SimpleCounter::SampleStackCount, ini, simpleCounter,
          "SampleStackCount");
      Bind(SimpleCounter::SampleStackDepth, ini, simpleCounter,
          "SampleStackDepth");
    }
  }
  {
    Hdf stats = config["Stats"];
    Bind(EnableStats, ini, config, "Stats"); // main switch

    Bind(EnableWebStats, ini, stats, "Web");
    Bind(EnableMemoryStats, ini, stats, "Memory");
    Bind(EnableMemcacheStats, ini, stats, "Memcache");
    Bind(EnableMemcacheKeyStats, ini, stats, "MemcacheKey");
    Bind(EnableSQLStats, ini, stats, "SQL");
    Bind(EnableSQLTableStats, ini, stats, "SQLTable");
    Bind(EnableNetworkIOStatus, ini, stats, "NetworkIO");

    Bind(StatsXSL, ini, stats, "XSL");
    Bind(StatsXSLProxy, ini, stats, "XSLProxy");

    Bind(StatsSlotDuration, ini, stats, "SlotDuration");
    Bind(StatsMaxSlot, ini, stats, "MaxSlot");

    Bind(EnableHotProfiler, ini, stats, "EnableHotProfiler");
    Bind(ProfilerTraceBuffer, ini, stats, "ProfilerTraceBuffer");
    Bind(ProfilerTraceExpansion, ini, stats, "ProfilerTraceExpansion");
    Bind(ProfilerMaxTraceBuffer, ini, stats, "ProfilerMaxTraceBuffer");
  }
  {
    Bind(ServerVariables, ini, config, "ServerVariables");
    Bind(EnvVariables, ini, config, "EnvVariables");
  }
  {
    Hdf eval = config["Eval"];
    Bind(EnableHipHopSyntax, ini, eval, "EnableHipHopSyntax");
    Bind(EnableHipHopExperimentalSyntax, ini, eval,
        "EnableHipHopExperimentalSyntax");
    Bind(EnableShortTags, ini, eval, "EnableShortTags");
    Bind(EnableAspTags, ini, eval, "EnableAspTags");
    Bind(EnableXHP, ini, eval, "EnableXHP");
    Bind(EnableZendCompat, ini, eval, "EnableZendCompat");
    Bind(TimeoutsUseWallTime, ini, eval, "TimeoutsUseWallTime");

    if (EnableHipHopSyntax) {
      // If EnableHipHopSyntax is true, it forces EnableXHP to true
      // regardless of how it was set in the config
      EnableXHP = true;
    }

    Bind(EnableObjDestructCall, ini, eval, "EnableObjDestructCall");
    Bind(MaxUserFunctionId, ini, eval, "MaxUserFunctionId");
    Bind(CheckSymLink, ini, eval, "CheckSymLink");

    Bind(EnableAlternative, ini, eval, "EnableAlternative");

#define F(type, name, def) \
    Bind(Eval ## name, ini, eval, #name);
    EVALFLAGS()
#undef F
    Util::low_malloc_huge_pages(EvalMaxLowMemHugePages);
    EvalJitEnableRenameFunction = EvalJitEnableRenameFunction || !EvalJit;

    Bind(EnableEmitSwitch, ini, eval, "EnableEmitSwitch");
    Bind(EnableEmitterStats, ini, eval, "EnableEmitterStats");
    Bind(EnableInstructionCounts, ini, eval, "EnableInstructionCounts");
    Bind(RecordCodeCoverage, ini, eval, "RecordCodeCoverage");
    if (EvalJit && RecordCodeCoverage) {
      throw InvalidArgumentException(
        "code coverage", "Code coverage is not supported for Eval.Jit=true");
    }
    if (RecordCodeCoverage) CheckSymLink = true;
    Bind(CodeCoverageOutputFile, ini, eval, "CodeCoverageOutputFile");
    {
      Hdf debugger = eval["Debugger"];
      Bind(EnableDebugger, ini, debugger, "EnableDebugger");
      Bind(EnableDebuggerColor, ini, debugger, "EnableDebuggerColor");
      Bind(EnableDebuggerPrompt, ini, debugger, "EnableDebuggerPrompt");
      Bind(EnableDebuggerServer, ini, debugger, "EnableDebuggerServer");
      Bind(EnableDebuggerUsageLog, ini, debugger, "EnableDebuggerUsageLog");
      Bind(DebuggerServerPort, ini, debugger, "Port");
      Bind(DebuggerDisableIPv6, ini, debugger, "DisableIPv6");
      Bind(DebuggerDefaultSandboxPath, ini, debugger, "DefaultSandboxPath");
      Bind(DebuggerStartupDocument, ini, debugger, "StartupDocument");
      Bind(DebuggerSignalTimeout, ini, debugger, "SignalTimeout");

      Bind(DebuggerDefaultRpcPort, ini, debugger, "RPC.DefaultPort");
      Bind(DebuggerDefaultRpcAuth, ini, debugger, "RPC.DefaultAuth");
      Bind(DebuggerRpcHostDomain, ini, debugger, "RPC.HostDomain");
      Bind(DebuggerDefaultRpcTimeout, ini, debugger, "RPC.DefaultTimeout");
    }
    {
      Hdf repo = config["Repo"];
      {
        Hdf repoLocal = repo["Local"];
        // Repo.Local.Mode.
        Bind(RepoLocalMode, ini, repoLocal, "Mode");
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
        Bind(RepoLocalPath, ini, repoLocal, "Path");
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
        Bind(RepoCentralPath, ini, repoCentral, "Path");
      }
      {
        Hdf repoEval = repo["Eval"];
        // Repo.Eval.Mode.
        Bind(RepoEvalMode, ini, repoEval, "Mode");
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
      Bind(RepoJournal, ini, repo, "Journal");
      Bind(RepoCommit, ini, repo, "Commit");
      Bind(RepoDebugInfo, ini, repo, "DebugInfo");
      Bind(RepoAuthoritative, ini, repo, "Authoritative");
    }

    EnableArgsInBacktraces = !RepoAuthoritative;
    Bind(EnableArgsInBacktraces, ini, eval, "EnableArgsInBacktraces");
  }
  {
    Hdf sandbox = config["Sandbox"];
    Bind(SandboxMode, ini, sandbox, "SandboxMode");
    Bind(SandboxPattern, ini, sandbox, "Pattern");
    SandboxPattern = Util::format_pattern(SandboxPattern, true);
    Bind(SandboxHome, ini, sandbox, "Home");
    Bind(SandboxFallback, ini, sandbox, "Fallback");
    Bind(SandboxConfFile, ini, sandbox, "ConfFile");
    Bind(SandboxFromCommonRoot, ini, sandbox, "FromCommonRoot");
    Bind(SandboxDirectoriesRoot, ini, sandbox, "DirectoriesRoot");
    Bind(SandboxLogsRoot, ini, sandbox, "LogsRoot");
    Bind(SandboxServerVariables, ini, sandbox, "ServerVariables");
  }
  {
    Hdf mail = config["Mail"];
    Bind(SendmailPath, ini, mail, "SendmailPath");
    Bind(MailForceExtraParameters, ini, mail, "ForceExtraParameters");
  }
  {
    Hdf preg = config["Preg"];
    Bind(PregBacktraceLimit, ini, preg, "BacktraceLimit");
    Bind(PregRecursionLimit, ini, preg, "RecursionLimit");
    Bind(EnablePregErrorLog, ini, preg, "ErrorLog");
  }
  {
    Hdf hhprofServer = config["HHProfServer"];
    Bind(HHProfServerEnabled, ini, hhprofServer, "Enabled");
    Bind(HHProfServerPort, ini, hhprofServer, "Port");
    Bind(HHProfServerThreads, ini, hhprofServer, "Threads");
    Bind(HHProfServerTimeoutSeconds, ini, hhprofServer, "TimeoutSeconds");
    Bind(HHProfServerProfileClientMode, ini, hhprofServer, "ProfileClientMode");
    Bind(HHProfServerAllocationProfile, ini, hhprofServer, "AllocationProfile");

    // HHProfServer.Filter.*
    Hdf hhprofFilter = hhprofServer["Filter"];
    Bind(HHProfServerFilterMinAllocPerReq, ini, hhprofFilter, "MinAllocPerReq");
    Bind(HHProfServerFilterMinBytesPerReq, ini, hhprofFilter, "MinBytesPerReq");
  }
  {
    Hdf simplexml = config["SimpleXML"];
    Bind(SimpleXMLEmptyNamespaceMatchesAll, ini, simplexml,
        "EmptyNamespaceMatchesAll");
  }
#ifdef FACEBOOK
  {
    Hdf fb303Server = config["Fb303Server"];
    Bind(EnableFb303Server, ini, fb303Server, "Enable");
    Bind(Fb303ServerPort, ini, fb303Server, "Port");
    Bind(Fb303ServerThreadStackSizeMb, ini, fb303Server, "ThreadStackSizeMb");
    Bind(Fb303ServerWorkerThreads, ini, fb303Server, "WorkerThreads");
    Bind(Fb303ServerPoolThreads, ini, fb303Server, "PoolThreads");
  }
#endif

  refineStaticStringTableSize();

  auto ext = IniSetting::CORE;

  // Language and Misc Configuration Options
  IniSetting::Bind(ext, IniSetting::PHP_INI_ONLY, "expose_php", &ExposeHPHP);

  // Data Handling
  IniSetting::Bind(ext, IniSetting::PHP_INI_PERDIR,
                   "always_populate_raw_post_data",
                   &RuntimeOption::AlwaysPopulateRawPostData);
  IniSetting::Bind(ext, IniSetting::PHP_INI_PERDIR, "post_max_size",
                   ini_on_update_long,
                   [](void*) {
                     return std::to_string(VirtualHost::GetMaxPostSize());
                   },
                   &RuntimeOption::MaxPostSize);

  // Paths and Directories
  IniSetting::Bind(ext, IniSetting::PHP_INI_SYSTEM, "doc_root",
                   &RuntimeOption::SourceRoot);

  // FastCGI
  IniSetting::Bind(ext, IniSetting::PHP_INI_ONLY, "pid",
                   &RuntimeOption::PidFile);

  // File Uploads
  IniSetting::Bind(ext, IniSetting::PHP_INI_SYSTEM, "file_uploads", "true",
                   &RuntimeOption::EnableFileUploads);
  IniSetting::Bind(ext, IniSetting::PHP_INI_SYSTEM, "upload_tmp_dir",
                   &RuntimeOption::UploadTmpDir);
  IniSetting::Bind(ext, IniSetting::PHP_INI_PERDIR, "upload_max_filesize",
                   ini_on_update_long,
                   [](void*) {
                     int uploadMaxFilesize =
                       VirtualHost::GetUploadMaxFileSize() / (1 << 20);
                     return std::to_string(uploadMaxFilesize) + "M";
                   },
                   &RuntimeOption::UploadMaxFileSize);

  // Filesystem and Streams Configuration Options
  IniSetting::Bind(ext, IniSetting::PHP_INI_SYSTEM, "allow_url_fopen",
                   ini_on_update_fail, ini_get_static_string_1);

  // HPHP specific
  IniSetting::Bind(ext, IniSetting::PHP_INI_NONE, "hphp.compiler_id",
                   ini_on_update_fail,
                   [](void*) {
                     return getHphpCompilerId();
                   });
  IniSetting::Bind(ext, IniSetting::PHP_INI_NONE, "hphp.compiler_version",
                   ini_on_update_fail,
                   [](void*) {
                     return getHphpCompilerVersion();
                   });
  IniSetting::Bind(ext, IniSetting::PHP_INI_NONE, "hhvm.ext_zend_compat",
                   ini_on_update_fail, ini_get_bool,
                   &RuntimeOption::EnableZendCompat),
  IniSetting::Bind(ext, IniSetting::PHP_INI_NONE, "hphp.build_id",
                   ini_on_update_fail, ini_get_stdstring,
                   &RuntimeOption::BuildId);
  IniSetting::Bind(ext, IniSetting::PHP_INI_SYSTEM, "notice_frequency",
                   &RuntimeOption::NoticeFrequency);
  IniSetting::Bind(ext, IniSetting::PHP_INI_SYSTEM, "warning_frequency",
                   &RuntimeOption::WarningFrequency);

  Extension::LoadModules(config, ini);
  if (overwrites) Loaded = true;
}

std::string RuntimeOption::Normalize(const std::string &name) {
  std::string out = "";
  bool start = true;
  bool supress_next_underscore = false;
  for (auto &c : name) {
    if (start) {
      out += ".";
      out += tolower(c);
      start = false;
      supress_next_underscore = true;
    } else if (!isalpha(c)) {
      out += c;
      supress_next_underscore = true;
    } else if (isupper(c)) {
      if (!supress_next_underscore) {
        out += "_";
      }
      out += tolower(c);
      supress_next_underscore = true;
    } else {
      out += c;
      supress_next_underscore = false;
    }
  }
  return out;
}

std::string RuntimeOption::IniName(const Hdf& config, const char* name) {
  return "hhvm" + Normalize(config.getFullPath()) + Normalize(name);
}

static void StringInsert(std::vector<std::string> &values,
                         const std::string &key,
                         const std::string &value) {
  values.push_back(value);
}
static void StringInsert(boost::container::flat_set<std::string> &values,
                         const std::string &key,
                         const std::string &value) {
  values.insert(value);
}
static void StringInsert(std::set<std::string, stdltistr> &values,
                         const std::string &key,
                         const std::string &value) {
  values.insert(value);
}
static void StringInsert(std::set<std::string> &values,
                         const std::string &key,
                         const std::string &value) {
  values.insert(value);
}
static void StringInsert(std::map<std::string, std::string> &values,
                         const std::string &key,
                         const std::string &value) {
  values[key] = value;
}
static void StringInsert(hphp_string_imap<std::string> &values,
                         const std::string &key,
                         const std::string &value) {
  values[key] = value;
}

template<class T>
void RuntimeOption::Get(const IniSetting::Map &ini,
                        const Hdf& config,
                        const char* name,
                        T &data) {
  data.clear();
  for (Hdf hdf = config[name].firstChild(); hdf.exists(); hdf = hdf.next()) {
    StringInsert(data, hdf.getName(), hdf.getString());
  }
  if (!data.empty()) {
    return;
  }
  auto key = IniName(config, name);
  auto* value = ini.get_ptr(key);
  if (!value || !value->isArray()) {
    return;
  }
  for (auto &pair : value->items()) {
    StringInsert(data, pair.first.asString().toStdString(),
                       pair.second.asString().toStdString());
  }
}

const char* RuntimeOption::Get(const IniSetting::Map &ini,
                               const Hdf& config,
                               const char* name) {
  auto data = config[name].get();
  if (data != nullptr) {
    return data;
  }
  auto key = IniName(config, name);
  auto* value = ini.get_ptr(key);
  if (!value || !value->isString()) {
    return nullptr;
  }
  return value->data();
}

bool RuntimeOption::GetBool(const IniSetting::Map &ini,
                            const Hdf& config,
                            const char* name,
                            bool defValue) {
  auto data = Get(ini, config, name);
  if (data == nullptr) {
    return defValue;
  }
  bool ret;
  ini_on_update_bool(data, &ret);
  return ret;
}

int64_t RuntimeOption::GetInt(const IniSetting::Map &ini,
                              const Hdf& config,
                              const char* name,
                              int64_t defValue) {
  auto data = Get(ini, config, name);
  if (data == nullptr) {
    return defValue;
  }
  int64_t ret;
  ini_on_update_long(data, &ret);
  return ret;
}

std::string RuntimeOption::GetString(const IniSetting::Map &ini,
                                     const Hdf& config,
                                     const char* name,
                                     std::string defValue) {
  auto data = Get(ini, config, name);
  if (data == nullptr) {
    return defValue;
  }
  return data;
}

template<class T>
void RuntimeOption::BindToIni(const Hdf& config, const char* name,
                              T &location) {
  IniSetting::Bind(
    IniSetting::CORE, IniSetting::PHP_INI_SYSTEM,
    IniName(config, name).c_str(), &location
  );
}

template<class T>
void RuntimeOption::Bind(T &a,
                         const IniSetting::Map &ini,
                         const Hdf& config,
                         const char* name) {
  T data;
  // No call to BindToIni since we don't have ini_on_update_crazy_set_thing yet
  Get(ini, config, name, data);
  if (!data.empty()) {
    a = data;
  }
}

void RuntimeOption::Bind(std::string &a,
                         const IniSetting::Map &ini,
                         const Hdf& config,
                         const char* name) {
  BindToIni(config, name, a);
  auto data = Get(ini, config, name);
  if (data != nullptr) {
    a = data;
  }
}

void RuntimeOption::Bind(bool &a,
                         const IniSetting::Map &ini,
                         const Hdf& config,
                         const char* name) {
  BindToIni(config, name, a);
  auto data = Get(ini, config, name);
  if (data != nullptr) {
    ini_on_update_bool(data, &a);
  }
}

void RuntimeOption::Bind(int16_t &a,
                         const IniSetting::Map &ini,
                         const Hdf& config,
                         const char* name) {
  BindToIni(config, name, a);
  auto data = Get(ini, config, name);
  if (data != nullptr) {
    ini_on_update_short(data, &a);
  }
}

void RuntimeOption::Bind(int32_t &a,
                         const IniSetting::Map &ini,
                         const Hdf& config,
                         const char* name) {
  BindToIni(config, name, a);
  auto data = Get(ini, config, name);
  if (data != nullptr) {
    ini_on_update_int(data, &a);
  }
}

void RuntimeOption::Bind(int64_t &a,
                         const IniSetting::Map &ini,
                         const Hdf& config,
                         const char* name) {
  BindToIni(config, name, a);
  auto data = Get(ini, config, name);
  if (data != nullptr) {
    ini_on_update_long(data, &a);
  }
}

void RuntimeOption::Bind(uint16_t &a,
                         const IniSetting::Map &ini,
                         const Hdf& config,
                         const char* name) {
  BindToIni(config, name, a);
  auto data = Get(ini, config, name);
  if (data != nullptr) {
    ini_on_update_short(data, &a);
  }
}

void RuntimeOption::Bind(uint32_t &a,
                         const IniSetting::Map &ini,
                         const Hdf& config,
                         const char* name) {
  BindToIni(config, name, a);
  auto data = Get(ini, config, name);
  if (data != nullptr) {
    ini_on_update_int(data, &a);
  }
}

void RuntimeOption::Bind(uint64_t &a,
                         const IniSetting::Map &ini,
                         const Hdf& config,
                         const char* name) {
  BindToIni(config, name, a);
  auto data = Get(ini, config, name);
  if (data != nullptr) {
    ini_on_update_long(data, &a);
  }
}

void RuntimeOption::Bind(double &a,
                         const IniSetting::Map &ini,
                         const Hdf& config,
                         const char* name) {
  BindToIni(config, name, a);
  auto data = Get(ini, config, name);
  if (data != nullptr) {
    ini_on_update_real(data, &a);
  }
}

void RuntimeOption::Bind(const IniSetting::Map &ini, const Hdf& config, const
                         char *name, UpdateCallback updateCallback,
                         GetCallback getCallback, void *p) {
  IniSetting::Bind(
    IniSetting::CORE, IniSetting::PHP_INI_SYSTEM,
    IniName(config, name).c_str(), updateCallback, getCallback, p
  );
  auto data = Get(ini, config, name);
  if (data != nullptr) {
    updateCallback(data, p);
  }
}

void RuntimeOption::SetResourceLimit(int resource, const IniSetting::Map &ini,
                                     Hdf rlimit, const char *nodeName) {
  auto data = Get(ini, rlimit, nodeName);
  if (data != nullptr) {
    IniSetting::Bind(
      IniSetting::CORE, IniSetting::PHP_INI_PERDIR,
      IniName(rlimit, nodeName).c_str(), data,
      [=](const std::string &value, void*) {
        struct rlimit rl;
        getrlimit(resource, &rl);
        if (rl.rlim_max < rl.rlim_cur) {
          rl.rlim_max = rl.rlim_cur;
        }
        int ret = setrlimit(resource, &rl);
        if (ret) {
          Logger::Error("Unable to set %s to %" PRId64 ": %s (%d)",
                        nodeName, (uint64_t)rl.rlim_cur,
                        folly::errnoStr(errno).c_str(), errno);
        }
        return !ret;
      },
      [=](void*) {
        struct rlimit rl;
        getrlimit(resource, &rl);
        return std::to_string(rl.rlim_cur);
      }
    );
  }
}

///////////////////////////////////////////////////////////////////////////////
}
