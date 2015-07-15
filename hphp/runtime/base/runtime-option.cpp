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

#include <folly/String.h>

#include "hphp/util/hdf.h"
#include "hphp/util/text-util.h"
#include "hphp/util/network.h"
#include "hphp/util/hardware-counter.h"
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
#include "hphp/runtime/base/apc-file-storage.h"
#include "hphp/runtime/base/extended-logger.h"
#include "hphp/runtime/base/simple-counter.h"
#include "hphp/runtime/base/memory-manager.h"
#include "hphp/runtime/base/preg.h"
#include "hphp/runtime/base/crash-reporter.h"
#include "hphp/runtime/base/static-string-table.h"
#include "hphp/runtime/base/config.h"
#include "hphp/runtime/base/ini-setting.h"
#include "hphp/runtime/base/hphp-system.h"
#include "hphp/runtime/base/zend-url.h"
#include "hphp/runtime/ext/extension-registry.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

const char *RuntimeOption::ExecutionMode = "";
std::string RuntimeOption::BuildId;
std::string RuntimeOption::InstanceId;
std::string RuntimeOption::PidFile = "www.pid";

bool RuntimeOption::EnableHipHopSyntax = false;
bool RuntimeOption::EnableHipHopExperimentalSyntax = false;
bool RuntimeOption::EnableShortTags = true;
bool RuntimeOption::EnableAspTags = false;
bool RuntimeOption::EnableXHP = false;
bool RuntimeOption::EnableObjDestructCall = true;
bool RuntimeOption::EnableEmitSwitch = true;
bool RuntimeOption::EnableEmitterStats = true;
bool RuntimeOption::EnableIntrinsicsExtension = false;
bool RuntimeOption::CheckSymLink = true;
bool RuntimeOption::EnableArgsInBacktraces = true;
bool RuntimeOption::EnableZendCompat = false;
bool RuntimeOption::EnableZendSorting = false;
bool RuntimeOption::EnableZendIniCompat = true;
bool RuntimeOption::TimeoutsUseWallTime = true;
bool RuntimeOption::CheckFlushOnUserClose = true;
bool RuntimeOption::EvalAuthoritativeMode = false;
bool RuntimeOption::IntsOverflowToInts = false;

std::string RuntimeOption::LogFile;
std::string RuntimeOption::LogFileSymLink;
int RuntimeOption::LogHeaderMangle = 0;
bool RuntimeOption::AlwaysLogUnhandledExceptions =
  RuntimeOption::EnableHipHopSyntax;
bool RuntimeOption::AlwaysEscapeLog = true;
bool RuntimeOption::NoSilencer = false;
int RuntimeOption::ErrorUpgradeLevel = 0;
bool RuntimeOption::CallUserHandlerOnFatals = false;
bool RuntimeOption::ThrowExceptionOnBadMethodCall = true;
bool RuntimeOption::LogNativeStackOnOOM = true;
int RuntimeOption::RuntimeErrorReportingLevel =
  static_cast<int>(ErrorMode::HPHP_ALL);
int RuntimeOption::ForceErrorReportingLevel = 0;

std::string RuntimeOption::ServerUser;

int RuntimeOption::MaxLoopCount = 0;
int RuntimeOption::MaxSerializedStringSize = 64 * 1024 * 1024; // 64MB
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
std::map<std::string, AccessLogFileData> RuntimeOption::AccessLogs;

std::string RuntimeOption::AdminLogFormat = "%h %t %s %U";
std::string RuntimeOption::AdminLogFile;
std::string RuntimeOption::AdminLogSymLink;

std::map<std::string, AccessLogFileData> RuntimeOption::RPCLogs;

std::string RuntimeOption::Host;
std::string RuntimeOption::DefaultServerNameSuffix;
std::string RuntimeOption::ServerType = "libevent";
std::string RuntimeOption::ServerIP;
std::string RuntimeOption::ServerFileSocket;
int RuntimeOption::ServerPort = 80;
int RuntimeOption::ServerPortFd = -1;
int RuntimeOption::ServerBacklog = 128;
int RuntimeOption::ServerConnectionLimit = 0;
int RuntimeOption::ServerThreadCount = 50;
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
bool RuntimeOption::ServerAddVaryEncoding = true;
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
int RuntimeOption::PspCpuTimeoutSeconds = 0;
int64_t RuntimeOption::MaxRequestAgeFactor = 0;
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
int RuntimeOption::GzipMaxCompressionLevel = 9;
std::string RuntimeOption::ForceCompressionURL;
std::string RuntimeOption::ForceCompressionCookie;
std::string RuntimeOption::ForceCompressionParam;
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
bool RuntimeOption::AlwaysPopulateRawPostData = false;
int64_t RuntimeOption::UploadMaxFileSize = 100;
std::string RuntimeOption::UploadTmpDir = "/tmp";
bool RuntimeOption::EnableFileUploads = true;
bool RuntimeOption::EnableUploadProgress = false;
int64_t RuntimeOption::MaxFileUploads = 20;
int RuntimeOption::Rfc1867Freq = 256 * 1024;
std::string RuntimeOption::Rfc1867Prefix = "vupload_";
std::string RuntimeOption::Rfc1867Name = "video_ptoken";
bool RuntimeOption::LibEventSyncSend = true;
bool RuntimeOption::ExpiresActive = true;
int RuntimeOption::ExpiresDefault = 2592000;
std::string RuntimeOption::DefaultCharsetName = "";
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

std::string RuntimeOption::ProxyOriginRaw;
int RuntimeOption::ProxyPercentageRaw = 0;
int RuntimeOption::ProxyRetry = 3;
bool RuntimeOption::UseServeURLs;
std::set<std::string> RuntimeOption::ServeURLs;
bool RuntimeOption::UseProxyURLs;
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
std::string RuntimeOption::StackTraceFilename;
bool RuntimeOption::LocalMemcache = false;
bool RuntimeOption::MemcacheReadOnly = false;
int RuntimeOption::StackTraceTimeout = 0; // seconds; 0 means unlimited

bool RuntimeOption::EnableStats = false;
bool RuntimeOption::EnableAPCStats = false;
bool RuntimeOption::EnableWebStats = false;
bool RuntimeOption::EnableMemoryStats = false;
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
int64_t RuntimeOption::SocketDefaultTimeout = 60;
bool RuntimeOption::LockCodeMemory = false;
int RuntimeOption::MaxArrayChain = INT_MAX;
bool RuntimeOption::WarnOnCollectionToArray = false;
bool RuntimeOption::UseDirectCopy = false;

#ifdef FOLLY_SANITIZE_ADDRESS
bool RuntimeOption::DisableSmallAllocator = true;
#else
bool RuntimeOption::DisableSmallAllocator = false;
#endif

std::map<std::string, std::string> RuntimeOption::ServerVariables;
std::map<std::string, std::string> RuntimeOption::EnvVariables;

std::string RuntimeOption::LightProcessFilePrefix = "./lightprocess";
int RuntimeOption::LightProcessCount = 0;

int64_t RuntimeOption::HeapSizeMB = 4096; // 4gb
int64_t RuntimeOption::HeapResetCountBase = 1;
int64_t RuntimeOption::HeapResetCountMultiple = 2;
int64_t RuntimeOption::HeapLowWaterMark = 16;
int64_t RuntimeOption::HeapHighWaterMark = 1024;

#ifdef HHVM_DYNAMIC_EXTENSION_DIR
std::string RuntimeOption::ExtensionDir = HHVM_DYNAMIC_EXTENSION_DIR;
#else
std::string RuntimeOption::ExtensionDir = "";
#endif

std::vector<std::string> RuntimeOption::Extensions;
std::vector<std::string> RuntimeOption::DynamicExtensions;
std::string RuntimeOption::DynamicExtensionPath = ".";

HackStrictOption
  RuntimeOption::StrictArrayFillKeys = HackStrictOption::OFF,
  RuntimeOption::DisallowDynamicVarEnvFuncs = HackStrictOption::OFF,
  RuntimeOption::IconvIgnoreCorrect = HackStrictOption::OFF,
  RuntimeOption::MinMaxAllowDegenerate = HackStrictOption::OFF;
bool RuntimeOption::LookForTypechecker = true;
bool RuntimeOption::AutoTypecheck = true;

int RuntimeOption::GetScannerType() {
  int type = 0;
  if (EnableShortTags) type |= Scanner::AllowShortTags;
  if (EnableAspTags) type |= Scanner::AllowAspTags;
  if (EnableXHP) type |= Scanner::AllowXHPSyntax;
  if (EnableHipHopSyntax) type |= Scanner::AllowHipHopSyntax;
  return type;
}

const std::string& RuntimeOption::GetServerPrimaryIPv4() {
   static std::string serverPrimaryIPv4 = GetPrimaryIPv4();
   return serverPrimaryIPv4;
}

const std::string& RuntimeOption::GetServerPrimaryIPv6() {
   static std::string serverPrimaryIPv6 = GetPrimaryIPv6();
   return serverPrimaryIPv6;
}

static inline std::string regionSelectorDefault() {
  return "tracelet";
}

static inline bool pgoDefault() {
  // TODO(3496304)
  return !RuntimeOption::EvalSimulateARM;
}

static inline uint64_t pgoThresholdDefault() {
#ifdef HHVM_WHOLE_CFG
  return 100;
#else
  return debug ? 2 : 100;
#endif
}

static inline std::string pgoRegionSelectorDefault() {
#ifdef HHVM_WHOLE_CFG
  return "wholecfg";
#else
  return "hotcfg";
#endif
}

static inline bool loopsDefault() {
#ifdef HHVM_JIT_LOOPS_BY_DEFAULT
  return true;
#else
  return (RuntimeOption::EvalJitPGORegionSelector == "wholecfg" ||
          RuntimeOption::EvalJitPGORegionSelector == "hotcfg");
#endif
}

static inline bool hhirConstrictGuardsDefault() {
#ifdef HHVM_CONSTRICT_GUARDS_BY_DEFAULT
  return true;
#else
  return false;
#endif
}

static inline bool hhirRelaxGuardsDefault() {
  return !RuntimeOption::EvalHHIRConstrictGuards;
}

static inline bool evalJitDefault() {
#ifdef __CYGWIN__
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

/*
 * 0: never use LLVM
 * 1: use LLVM for TransOptimize translations
 * 2: use LLVM for all translations
 */
static inline uint32_t jitLLVMDefault() {
#ifdef HHVM_JIT_LLVM_BY_DEFAULT
  return 2;
#else
  return 0;
#endif
}

static inline bool jitLLVMSLPVectorizeDefault() {
  return RuntimeOption::EvalJitLLVMOptLevel > 1 &&
         RuntimeOption::EvalJitLLVMSizeLevel < 2;
}

static inline bool reuseTCDefault() {
  return hhvm_reuse_tc && !RuntimeOption::RepoAuthoritative;
}

static inline bool hugePagesSoundNice() {
  return RuntimeOption::ServerExecutionMode();
}

static inline int nsjrDefault() {
  return RuntimeOption::ServerExecutionMode() ? 5 : 0;
}

uint64_t ahotDefault() {
  return RuntimeOption::RepoAuthoritative ? 4 << 20 : 0;
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
static const uint32_t kDefaultProfileRequests = debug ? 1 << 31 : 500;
static const size_t kJitGlobalDataDef = RuntimeOption::EvalJitASize >> 2;
static const uint64_t kJitRelocationSizeDefault = 1 << 20;

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
bool RuntimeOption::EnableFb303Server = false;
int RuntimeOption::Fb303ServerPort = 0;
int RuntimeOption::Fb303ServerThreadStackSizeMb = 8;
int RuntimeOption::Fb303ServerWorkerThreads = 1;
int RuntimeOption::Fb303ServerPoolThreads = 1;
#endif

double RuntimeOption::XenonPeriodSeconds = 0.0;
bool RuntimeOption::XenonForceAlwaysOn = false;

std::map<std::string, std::string> RuntimeOption::CustomSettings;

int RuntimeOption::EnableAlternative = 0;

#ifdef NDEBUG
  #ifdef ALWAYS_ASSERT
    const StaticString s_hhvm_build_type("Release with asserts");
  #else
    const StaticString s_hhvm_build_type("Release");
  #endif
#else
  const StaticString s_hhvm_build_type("Debug");
#endif

///////////////////////////////////////////////////////////////////////////////

static void setResourceLimit(int resource, const IniSetting::Map& ini,
                             const Hdf& rlimit, const char* nodeName) {
  if (!Config::GetString(ini, rlimit, nodeName).empty()) {
    struct rlimit rl;
    getrlimit(resource, &rl);
    rl.rlim_cur = Config::GetInt64(ini, rlimit, nodeName);
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

static bool matchHdfPattern(const std::string &value,
                            const IniSetting::Map& ini, Hdf hdfPattern,
                            const std::string& name) {
  string pattern = Config::GetString(ini, hdfPattern, name, "", false);
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

// A machine can belong to a tier, which can overwrite
// various settings, even if they are set in the same
// hdf file. However, CLI overrides still win the day over
// everything.
static std::vector<std::string> getTierOverwrites(IniSetting::Map& ini,
                                                  Hdf& config) {

  // Machine metrics
  string hostname, tier, cpu;
  {
    hostname = Config::GetString(ini, config, "Machine.name");
    if (hostname.empty()) {
      hostname = Process::GetHostName();
    }

    tier = Config::GetString(ini, config, "Machine.tier");

    cpu = Config::GetString(ini, config, "Machine.cpu");
    if (cpu.empty()) {
      cpu = Process::GetCPUModel();
    }
  }

  std::vector<std::string> messages;
  // Tier overwrites
  {
    for (Hdf hdf = config["Tiers"].firstChild(); hdf.exists();
         hdf = hdf.next()) {
      if (messages.empty()) {
        messages.emplace_back(folly::sformat(
                                "Matching tiers using: "
                                "machine='{}', tier='{}', cpu = '{}'",
                                hostname, tier, cpu));
      }
      if (matchHdfPattern(hostname, ini, hdf, "machine") &&
          matchHdfPattern(tier, ini, hdf, "tier") &&
          matchHdfPattern(cpu, ini, hdf, "cpu")) {
        messages.emplace_back(folly::sformat(
                                "Matched tier: {}", hdf.getName()));
        config.copy(hdf["overwrite"]);
        // no break here, so we can continue to match more overwrites
      }
      hdf["overwrite"].setVisited(); // avoid lint complaining
    }
  }
  return messages;
}


void RuntimeOption::Load(
  IniSetting::Map& ini, Hdf& config,
  const std::vector<std::string>& iniClis /* = std::vector<std::string>() */,
  const std::vector<std::string>& hdfClis /* = std::vector<std::string>() */,
  std::vector<std::string>* messages /* = nullptr */) {

  // Intialize the memory manager here because various settings and
  // initializations that we do here need it
  MemoryManager::TlsWrapper::getCheck();

  // Get the ini (-d) and hdf (-v) strings, which may override some
  // of options that were set from config files. We also do these
  // now so we can override Tier.*.[machine | tier | cpu] on the
  // command line, along with any fields within a Tier (e.g.,
  // CoreFileSize)
  for (auto& istr : iniClis) {
    Config::ParseIniString(istr, ini);
  }
  for (auto& hstr : hdfClis) {
    Config::ParseHdfString(hstr, config);
  }
  // See if there are any Tier-based overrides
  auto m = getTierOverwrites(ini, config);
  if (messages) *messages = std::move(m);

  // Then get the ini and hdf cli strings again, in case the tier overwrites
  // overrode any non-tier based command line option we set. The tier-based
  // command line overwrites will already have been set in the call above.
  // This extra call is for the other command line options that may have been
  // overridden by a tier, but shouldn't have been.
  for (auto& istr : iniClis) {
    Config::ParseIniString(istr, ini);
  }
  for (auto& hstr : hdfClis) {
    Config::ParseHdfString(hstr, config);
  }

  Config::Bind(PidFile, ini, config, "PidFile", "www.pid");

  Config::Bind(DynamicInvokeFunctions, ini, config, "DynamicInvokeFunctions");

  {
    // Logging
    auto setLogLevel = [](const std::string& value) {
      // ini parsing treats "None" as ""
      if (value == "None" || value == "") {
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
    };
    auto str = Config::GetString(ini, config, "Log.Level");
    if (!str.empty()) {
      setLogLevel(str);
    }
    IniSetting::Bind(IniSetting::CORE, IniSetting::PHP_INI_SYSTEM,
                     "hhvm.log.level", IniSetting::SetAndGet<std::string>(
      setLogLevel,
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

    Config::Bind(Logger::LogHeader, ini, config, "Log.Header");
    if (Config::GetBool(ini, config, "Log.AlwaysPrintStackTraces")) {
      Logger::SetTheLogger(new ExtendedLogger());
      ExtendedLogger::EnabledByDefault = true;
    }
    Config::Bind(Logger::LogNativeStackTrace, ini, config,
                 "Log.NativeStackTrace", true);
    Config::Bind(Logger::MaxMessagesPerRequest, ini,
                 config, "Log.MaxMessagesPerRequest", -1);

    Config::Bind(Logger::UseSyslog, ini, config, "Log.UseSyslog", false);
    Config::Bind(Logger::UseLogFile, ini, config, "Log.UseLogFile", true);
    Config::Bind(Logger::UseCronolog, ini, config, "Log.UseCronolog", false);
    Config::Bind(Logger::UseRequestLog, ini, config, "Log.UseRequestLog",
                 false);
    if (Logger::UseLogFile) {
      Config::Bind(LogFile, ini, config, "Log.File");
      if (!RuntimeOption::ServerExecutionMode()) {
        LogFile.clear();
      }
      if (LogFile[0] == '|') Logger::IsPipeOutput = true;
      Config::Bind(LogFileSymLink, ini, config, "Log.SymLink");
    }
    Config::Bind(LogFileFlusher::DropCacheChunkSize, ini,
                 config, "Log.DropCacheChunkSize", 1 << 20);
    Config::Bind(Logger::AlwaysEscapeLog, ini, config, "Log.AlwaysEscapeLog",
                 true);
    Config::Bind(RuntimeOption::LogHeaderMangle, ini, config,
                 "Log.HeaderMangle", 0);
    Config::Bind(AlwaysLogUnhandledExceptions, ini,
                 config, "Log.AlwaysLogUnhandledExceptions",
                 RuntimeOption::EnableHipHopSyntax);
    Config::Bind(NoSilencer, ini, config, "Log.NoSilencer");
    Config::Bind(RuntimeErrorReportingLevel, ini,
                config, "Log.RuntimeErrorReportingLevel",
                static_cast<int>(ErrorMode::HPHP_ALL));
    Config::Bind(ForceErrorReportingLevel, ini,
                 config, "Log.ForceErrorReportingLevel", 0);
    Config::Bind(AccessLogDefaultFormat, ini, config,
                 "Log.AccessLogDefaultFormat", "%h %l %u %t \"%r\" %>s %b");

    auto parseLogs = [](Hdf root, IniSetting::Map& ini, const char* name,
                        std::map<std::string, AccessLogFileData>& logs) {
      for (Hdf hdf = root[name].firstChild(); hdf.exists(); hdf = hdf.next()) {
        string logName = hdf.getName();
        string fname = Config::GetString(ini, hdf, "File", "", false);
        if (fname.empty()) {
          continue;
        }
        string symLink = Config::GetString(ini, hdf, "SymLink", "", false);
        string format = Config::GetString(ini, hdf, "Format",
          AccessLogDefaultFormat, false);

        logs[logName] = AccessLogFileData(fname, symLink, format);
      }
    };

    parseLogs(config, ini, "Log.Access", AccessLogs);
    RPCLogs = AccessLogs;
    parseLogs(config, ini, "Log.RPC", RPCLogs);

    Config::Bind(AdminLogFormat, ini, config, "Log.AdminLog.Format",
                 "%h %t %s %U");
    Config::Bind(AdminLogFile, ini, config, "Log.AdminLog.File");
    Config::Bind(AdminLogSymLink, ini, config, "Log.AdminLog.SymLink");
  }
  {
    // Error Handling

    Config::Bind(ErrorUpgradeLevel, ini, config, "ErrorHandling.UpgradeLevel",
                 0);
    Config::Bind(MaxSerializedStringSize, ini,
                 config, "ErrorHandling.MaxSerializedStringSize",
                 64 * 1024 * 1024);
    Config::Bind(CallUserHandlerOnFatals, ini,
                 config, "ErrorHandling.CallUserHandlerOnFatals", false);
    Config::Bind(ThrowExceptionOnBadMethodCall, ini,
                 config, "ErrorHandling.ThrowExceptionOnBadMethodCall", true);
    Config::Bind(LogNativeStackOnOOM, ini,
                 config, "ErrorHandling.LogNativeStackOnOOM", false);
    Config::Bind(MaxLoopCount, ini, config, "ErrorHandling.MaxLoopCount", 0);
    Config::Bind(NoInfiniteRecursionDetection, ini,
                 config, "ErrorHandling.NoInfiniteRecursionDetection");
    Config::Bind(WarnTooManyArguments, ini, config,
                 "ErrorHandling.WarnTooManyArguments");
    Config::Bind(EnableHipHopErrors, ini, config,
                 "ErrorHandling.EnableHipHopErrors", true);
    Config::Bind(AssertActive, ini, config, "ErrorHandling.AssertActive");
    Config::Bind(AssertWarning, ini, config, "ErrorHandling.AssertWarning");
    Config::Bind(NoticeFrequency, ini, config, "ErrorHandling.NoticeFrequency",
                 1);
    Config::Bind(WarningFrequency, ini, config,
                 "ErrorHandling.WarningFrequency", 1);
  }
  {
    Hdf rlimit = config["ResourceLimit"];
    if (Config::GetInt64(ini, config, "ResourceLimit.CoreFileSizeOverride")) {
      setResourceLimit(RLIMIT_CORE, ini,  config,
                       "ResourceLimit.CoreFileSizeOverride");
    } else {
      setResourceLimit(RLIMIT_CORE, ini, config, "ResourceLimit.CoreFileSize");
    }
    setResourceLimit(RLIMIT_NOFILE, ini, config, "ResourceLimit.MaxSocket");
    setResourceLimit(RLIMIT_DATA, ini, config, "ResourceLimit.RSS");
    // These don't have RuntimeOption::xxx bindings, but we still want to be
    // able to use ini_xxx functionality on them; so directly bind to a local
    // static via Config::Bind.
    static int64_t s_core_file_size_override, s_core_file_size, s_rss = 0;
    static int32_t s_max_socket = 0;
    Config::Bind(s_core_file_size_override, ini, config,
                 "ResourceLimit.CoreFileSizeOverride", 0);
    Config::Bind(s_core_file_size, ini, config, "ResourceLimit.CoreFileSize",
                 0);
    Config::Bind(s_max_socket, ini, config, "ResourceLimit.MaxSocket", 0);
    Config::Bind(s_rss, ini, config, "ResourceLimit.RSS", 0);

    Config::Bind(MaxRSS, ini, config, "ResourceLimit.MaxRSS", 0);
    Config::Bind(SocketDefaultTimeout, ini, config,
                 "ResourceLimit.SocketDefaultTimeout", 60);
    Config::Bind(MaxRSSPollingCycle, ini, config,
                 "ResourceLimit.MaxRSSPollingCycle", 0);
    Config::Bind(DropCacheCycle, ini, config, "ResourceLimit.DropCacheCycle",
                 0);
    Config::Bind(MaxSQLRowCount, ini, config, "ResourceLimit.MaxSQLRowCount",
                 0);
    Config::Bind(MaxMemcacheKeyCount, ini, config,
                 "ResourceLimit.MaxMemcacheKeyCount", 0);
    Config::Bind(SerializationSizeLimit, ini, config,
                 "ResourceLimit.SerializationSizeLimit", StringData::MaxSize);
    Config::Bind(StringOffsetLimit, ini, config,
                 "ResourceLimit.StringOffsetLimit", 10 * 1024 * 1024);
    Config::Bind(HeapSizeMB, ini, config, "ResourceLimit.HeapSizeMB",
                 HeapSizeMB);
    Config::Bind(HeapResetCountBase, ini, config,
                 "ResourceLimit.HeapResetCountBase", HeapResetCountBase);
    Config::Bind(HeapResetCountMultiple, ini, config,
                 "ResourceLimit.HeapResetCountMultiple",
                 HeapResetCountMultiple);
    Config::Bind(HeapLowWaterMark , ini, config,
                 "ResourceLimit.HeapLowWaterMark", HeapLowWaterMark);
    Config::Bind(HeapHighWaterMark , ini, config,
                 "ResourceLimit.HeapHighWaterMark",HeapHighWaterMark);
  }
  {
    // Repo
    {
      // Local Repo
      Config::Bind(RepoLocalMode, ini, config, "Repo.Local.Mode");
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
      Config::Bind(RepoLocalPath, ini, config, "Repo.Local.Path");
      if (RepoLocalPath.empty()) {
        const char* HHVM_REPO_LOCAL_PATH = getenv("HHVM_REPO_LOCAL_PATH");
        if (HHVM_REPO_LOCAL_PATH != nullptr) {
          RepoLocalPath = HHVM_REPO_LOCAL_PATH;
        }
      }
    }
    {
      // Central Repo
      Config::Bind(RepoCentralPath, ini, config, "Repo.Central.Path");
    }
    {
      // Repo - Eval
      Config::Bind(RepoEvalMode, ini, config, "Repo.Eval.Mode");
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
    Config::Bind(RepoJournal, ini, config, "Repo.Journal", "delete");
    Config::Bind(RepoCommit, ini, config, "Repo.Commit", true);
    Config::Bind(RepoDebugInfo, ini, config, "Repo.DebugInfo", true);
    Config::Bind(RepoAuthoritative, ini, config, "Repo.Authoritative", false);
    Config::Bind(RepoPreload, ini, config, "Repo.Preload", false);
  }
  {
    // Eval
    Config::Bind(EnableHipHopSyntax, ini, config, "Eval.EnableHipHopSyntax");
    Config::Bind(EnableHipHopExperimentalSyntax, ini,
                 config, "Eval.EnableHipHopExperimentalSyntax");
    Config::Bind(EnableShortTags, ini, config, "Eval.EnableShortTags", true);
    Config::Bind(EnableAspTags, ini, config, "Eval.EnableAspTags");
    Config::Bind(EnableXHP, ini, config, "Eval.EnableXHP", false);
    Config::Bind(EnableZendCompat, ini, config, "Eval.EnableZendCompat", false);
    Config::Bind(EnableZendSorting, ini, config, "Eval.EnableZendSorting",
                 false);
    Config::Bind(TimeoutsUseWallTime, ini, config, "Eval.TimeoutsUseWallTime",
                 true);
    Config::Bind(CheckFlushOnUserClose, ini, config,
                 "Eval.CheckFlushOnUserClose", true);

    if (EnableHipHopSyntax) {
      // If EnableHipHopSyntax is true, it forces EnableXHP to true
      // regardless of how it was set in the config
      EnableXHP = true;
    }

    Config::Bind(EnableObjDestructCall, ini, config,
                 "Eval.EnableObjDestructCall", true);
    Config::Bind(CheckSymLink, ini, config, "Eval.CheckSymLink", true);

    Config::Bind(EnableAlternative, ini, config, "Eval.EnableAlternative", 0);
#define F(type, name, defaultVal) \
    Config::Bind(Eval ## name, ini, config, "Eval."#name, defaultVal);
    EVALFLAGS()
#undef F
    if (EvalPerfRelocate > 0) {
      setRelocateRequests(EvalPerfRelocate);
    }
    low_malloc_huge_pages(EvalMaxLowMemHugePages);
    HardwareCounter::Init(EvalProfileHWEnable,
                          url_decode(EvalProfileHWEvents.data(),
                                     EvalProfileHWEvents.size()).toCppString(),
                          EvalRecordSubprocessTimes);

    Config::Bind(EnableEmitSwitch, ini, config, "Eval.EnableEmitSwitch", true);
    Config::Bind(EnableEmitterStats, ini, config, "Eval.EnableEmitterStats",
                 EnableEmitterStats);
    Config::Bind(EnableIntrinsicsExtension, ini,
                 config, "Eval.EnableIntrinsicsExtension",
                 EnableIntrinsicsExtension);
    Config::Bind(RecordCodeCoverage, ini, config, "Eval.RecordCodeCoverage");
    if (EvalJit && RecordCodeCoverage) {
      throw std::runtime_error("Code coverage is not supported with "
        "Eval.Jit=true");
    }
    Config::Bind(DisableSmallAllocator, ini, config,
                 "Eval.DisableSmallAllocator", DisableSmallAllocator);
    SetArenaSlabAllocBypass(DisableSmallAllocator);

    if (RecordCodeCoverage) CheckSymLink = true;
    Config::Bind(CodeCoverageOutputFile, ini, config,
                 "Eval.CodeCoverageOutputFile");
    // NB: after we know the value of RepoAuthoritative.
    Config::Bind(EnableArgsInBacktraces, ini, config,
                 "Eval.EnableArgsInBacktraces", !RepoAuthoritative);
    Config::Bind(EvalAuthoritativeMode, ini, config, "Eval.AuthoritativeMode",
                 false);
    if (RepoAuthoritative) {
      EvalAuthoritativeMode = true;
    }
    {
      // Debugger (part of Eval)
      Config::Bind(EnableDebugger, ini, config, "Eval.Debugger.EnableDebugger");
      Config::Bind(EnableDebuggerColor, ini, config,
                   "Eval.Debugger.EnableDebuggerColor", true);
      Config::Bind(EnableDebuggerPrompt, ini, config,
                   "Eval.Debugger.EnableDebuggerPrompt", true);
      Config::Bind(EnableDebuggerServer, ini, config,
                   "Eval.Debugger.EnableDebuggerServer");
      Config::Bind(EnableDebuggerUsageLog, ini, config,
                   "Eval.Debugger.EnableDebuggerUsageLog");
      Config::Bind(DebuggerServerPort, ini, config, "Eval.Debugger.Port", 8089);
      Config::Bind(DebuggerDisableIPv6, ini, config,
                   "Eval.Debugger.DisableIPv6", false);
      Config::Bind(DebuggerDefaultSandboxPath, ini, config,
                   "Eval.Debugger.DefaultSandboxPath");
      Config::Bind(DebuggerStartupDocument, ini, config,
                   "Eval.Debugger.StartupDocument");
      Config::Bind(DebuggerSignalTimeout, ini, config,
                   "Eval.Debugger.SignalTimeout", 1);
      Config::Bind(DebuggerDefaultRpcPort, ini, config,
                   "Eval.Debugger.RPC.DefaultPort", 8083);
      Config::Bind(DebuggerDefaultRpcAuth, ini, config,
                   "Eval.Debugger.RPC.DefaultAuth");
      Config::Bind(DebuggerRpcHostDomain, ini, config,
                   "Eval.Debugger.RPC.HostDomain");
      Config::Bind(DebuggerDefaultRpcTimeout, ini, config,
                   "Eval.Debugger.RPC.DefaultTimeout", 30);
    }
  }
  {
    // Hack Language
    Config::Bind(IntsOverflowToInts, ini, config,
                 "Hack.Lang.IntsOverflowToInts", EnableHipHopSyntax);
    Config::Bind(StrictArrayFillKeys, ini, config,
                 "Hack.Lang.StrictArrayFillKeys");
    Config::Bind(DisallowDynamicVarEnvFuncs, ini, config,
                 "Hack.Lang.DisallowDynamicVarEnvFuncs");
    Config::Bind(IconvIgnoreCorrect, ini, config,
                 "Hack.Lang.IconvIgnoreCorrect");
    Config::Bind(MinMaxAllowDegenerate, ini, config,
                 "Hack.Lang.MinMaxAllowDegenerate");
#ifdef FACEBOOK
    // Force off for Facebook unless you explicitly turn on; folks here both
    // disproportionately know what they are doing, and are doing work on HHVM
    // where this gets in the way.
    const bool aggroHackChecksDefault = false;
#else
    // Defaults to EnableHHSyntax since, if you have that on, you are
    // assumed to know what you're doing.
    const bool aggroHackChecksDefault = !EnableHipHopSyntax;
#endif
    Config::Bind(LookForTypechecker, ini, config,
                 "Hack.Lang.LookForTypechecker", aggroHackChecksDefault);
    Config::Bind(AutoTypecheck, ini, config, "Hack.Lang.AutoTypecheck",
                 aggroHackChecksDefault);
  }
  {
    // Server
    Config::Bind(Host, ini, config, "Server.Host");
    Config::Bind(DefaultServerNameSuffix, ini, config,
                 "Server.DefaultServerNameSuffix");
    Config::Bind(ServerType, ini, config, "Server.Type", ServerType);
    Config::Bind(ServerIP, ini, config, "Server.IP");
    Config::Bind(ServerFileSocket, ini, config, "Server.FileSocket");

#ifdef FACEBOOK
    //Do not cause slowness on startup -- except for Facebook
    if (GetServerPrimaryIPv4().empty() && GetServerPrimaryIPv6().empty()) {
      throw std::runtime_error("Unable to resolve the server's "
          "IPv4 or IPv6 address");
    }
#endif

    Config::Bind(ServerPort, ini, config, "Server.Port", 80);
    Config::Bind(ServerBacklog, ini, config, "Server.Backlog", 128);
    Config::Bind(ServerConnectionLimit, ini, config,
                 "Server.ConnectionLimit", 0);
    Config::Bind(ServerThreadCount, ini, config, "Server.ThreadCount",
                 Process::GetCPUCount() * 2);
    Config::Bind(ServerThreadRoundRobin, ini, config,
                 "Server.ThreadRoundRobin");
    Config::Bind(ServerWarmupThrottleRequestCount, ini, config,
                 "Server.WarmupThrottleRequestCount",
                 ServerWarmupThrottleRequestCount);
    Config::Bind(ServerThreadDropCacheTimeoutSeconds, ini, config,
                 "Server.ThreadDropCacheTimeoutSeconds", 0);
    if (Config::GetBool(ini, config, "Server.ThreadJobLIFO")) {
      ServerThreadJobLIFOSwitchThreshold = 0;
    }
    Config::Bind(ServerThreadJobLIFOSwitchThreshold, ini, config,
                 "Server.ThreadJobLIFOSwitchThreshold",
                 ServerThreadJobLIFOSwitchThreshold);
    Config::Bind(ServerThreadJobMaxQueuingMilliSeconds, ini, config,
                 "Server.ThreadJobMaxQueuingMilliSeconds", -1);
    Config::Bind(ServerThreadDropStack, ini, config, "Server.ThreadDropStack");
    Config::Bind(ServerHttpSafeMode, ini, config, "Server.HttpSafeMode");
    Config::Bind(ServerStatCache, ini, config, "Server.StatCache", false);
    Config::Bind(ServerFixPathInfo, ini, config, "Server.FixPathInfo", false);
    Config::Bind(ServerAddVaryEncoding, ini, config, "Server.AddVaryEncoding",
                 ServerAddVaryEncoding);
    Config::Bind(ServerWarmupRequests, ini, config, "Server.WarmupRequests");
    Config::Bind(ServerHighPriorityEndPoints, ini, config,
                 "Server.HighPriorityEndPoints");
    Config::Bind(ServerExitOnBindFail, ini, config, "Server.ExitOnBindFail",
                 false);

    Config::Bind(RequestTimeoutSeconds, ini, config,
                 "Server.RequestTimeoutSeconds", 0);
    Config::Bind(MaxRequestAgeFactor, ini, config, "Server.MaxRequestAgeFactor",
                 0);
    Config::Bind(PspTimeoutSeconds, ini, config, "Server.PspTimeoutSeconds", 0);
    Config::Bind(PspCpuTimeoutSeconds, ini, config,
                 "Server.PspCpuTimeoutSeconds", 0);
    Config::Bind(ServerMemoryHeadRoom, ini, config, "Server.MemoryHeadRoom", 0);
    Config::Bind(RequestMemoryMaxBytes, ini, config,
                 "Server.RequestMemoryMaxBytes", (16l << 30)); // 16GiB
    Config::Bind(ResponseQueueCount, ini, config, "Server.ResponseQueueCount",
                 0);
    if (ResponseQueueCount <= 0) {
      ResponseQueueCount = ServerThreadCount / 10;
      if (ResponseQueueCount <= 0) ResponseQueueCount = 1;
    }
    Config::Bind(ServerGracefulShutdownWait, ini,
                 config, "Server.GracefulShutdownWait", 0);
    Config::Bind(ServerHarshShutdown, ini, config, "Server.HarshShutdown",
                 true);
    Config::Bind(ServerKillOnSIGTERM, ini, config, "Server.KillOnSIGTERM",
                 false);
    Config::Bind(ServerEvilShutdown, ini, config, "Server.EvilShutdown", true);
    Config::Bind(ServerDanglingWait, ini, config, "Server.DanglingWait", 0);
    Config::Bind(ServerShutdownListenWait, ini, config,
                 "Server.ShutdownListenWait", 0);
    Config::Bind(ServerShutdownListenNoWork, ini, config,
                 "Server.ShutdownListenNoWork", -1);
    Config::Bind(ServerNextProtocols, ini, config, "Server.SSLNextProtocols");
    if (ServerGracefulShutdownWait < ServerDanglingWait) {
      ServerGracefulShutdownWait = ServerDanglingWait;
    }
    Config::Bind(GzipCompressionLevel, ini, config,
                 "Server.GzipCompressionLevel", 3);
    Config::Bind(GzipMaxCompressionLevel, ini, config,
                 "Server.GzipMaxCompressionLevel", 9);
    Config::Bind(ForceCompressionURL, ini, config,
                 "Server.ForceCompression.URL");
    Config::Bind(ForceCompressionCookie, ini, config,
                 "Server.ForceCompression.Cookie");
    Config::Bind(ForceCompressionParam, ini, config,
                 "Server.ForceCompression.Param");
    Config::Bind(EnableKeepAlive, ini, config, "Server.EnableKeepAlive", true);
    Config::Bind(ExposeHPHP, ini, config, "Server.ExposeHPHP", true);
    Config::Bind(ExposeXFBServer, ini, config, "Server.ExposeXFBServer", false);
    Config::Bind(ExposeXFBDebug, ini, config, "Server.ExposeXFBDebug", false);
    Config::Bind(XFBDebugSSLKey, ini, config, "Server.XFBDebugSSLKey", "");
    Config::Bind(ConnectionTimeoutSeconds, ini, config,
                 "Server.ConnectionTimeoutSeconds", -1);
    Config::Bind(EnableOutputBuffering, ini, config,
                 "Server.EnableOutputBuffering");
    Config::Bind(OutputHandler, ini, config, "Server.OutputHandler");
    Config::Bind(ImplicitFlush, ini, config, "Server.ImplicitFlush");
    Config::Bind(EnableEarlyFlush, ini, config, "Server.EnableEarlyFlush",
                 true);
    Config::Bind(ForceChunkedEncoding, ini, config,
                 "Server.ForceChunkedEncoding");
    Config::Bind(MaxPostSize, ini, config, "Server.MaxPostSize", 100);
    MaxPostSize <<= 20;
    Config::Bind(AlwaysPopulateRawPostData, ini, config,
                 "Server.AlwaysPopulateRawPostData", false);
    Config::Bind(LibEventSyncSend, ini, config, "Server.LibEventSyncSend",
                 true);
    Config::Bind(TakeoverFilename, ini, config, "Server.TakeoverFilename");
    Config::Bind(ExpiresActive, ini, config, "Server.ExpiresActive", true);
    Config::Bind(ExpiresDefault, ini, config, "Server.ExpiresDefault", 2592000);
    if (ExpiresDefault < 0) ExpiresDefault = 2592000;
    Config::Bind(DefaultCharsetName, ini, config, "Server.DefaultCharsetName",
                 "");
    Config::Bind(RequestBodyReadLimit, ini, config,
                 "Server.RequestBodyReadLimit", -1);
    Config::Bind(EnableSSL, ini, config, "Server.EnableSSL");
    Config::Bind(SSLPort, ini, config, "Server.SSLPort", 443);
    Config::Bind(SSLCertificateFile, ini, config, "Server.SSLCertificateFile");
    Config::Bind(SSLCertificateKeyFile, ini, config,
                 "Server.SSLCertificateKeyFile");
    Config::Bind(SSLCertificateDir, ini, config, "Server.SSLCertificateDir");
    Config::Bind(TLSDisableTLS1_2, ini, config, "Server.TLSDisableTLS1_2",
                 false);
    Config::Bind(TLSClientCipherSpec, ini, config,
                 "Server.TLSClientCipherSpec");

    string srcRoot = FileUtil::normalizeDir(
      Config::GetString(ini, config, "Server.SourceRoot"));
    if (!srcRoot.empty()) SourceRoot = srcRoot;
    FileCache::SourceRoot = SourceRoot;

    Config::Bind(IncludeSearchPaths, ini, config, "Server.IncludeSearchPaths");
    for (unsigned int i = 0; i < IncludeSearchPaths.size(); i++) {
      IncludeSearchPaths[i] = FileUtil::normalizeDir(IncludeSearchPaths[i]);
    }
    IncludeSearchPaths.insert(IncludeSearchPaths.begin(), ".");

    Config::Bind(FileCache, ini, config, "Server.FileCache");
    Config::Bind(DefaultDocument, ini, config, "Server.DefaultDocument");
    Config::Bind(ErrorDocument404, ini, config, "Server.ErrorDocument404");
    normalizePath(ErrorDocument404);
    Config::Bind(ForbiddenAs404, ini, config, "Server.ForbiddenAs404");
    Config::Bind(ErrorDocument500, ini, config, "Server.ErrorDocument500");
    normalizePath(ErrorDocument500);
    Config::Bind(FatalErrorMessage, ini, config, "Server.FatalErrorMessage");
    FontPath = FileUtil::normalizeDir(
      Config::GetString(ini, config, "Server.FontPath"));
    Config::Bind(EnableStaticContentFromDisk, ini, config,
                 "Server.EnableStaticContentFromDisk", true);
    Config::Bind(EnableOnDemandUncompress, ini, config,
                 "Server.EnableOnDemandUncompress", true);
    Config::Bind(EnableStaticContentMMap, ini, config,
                 "Server.EnableStaticContentMMap", true);
    if (EnableStaticContentMMap) {
      EnableOnDemandUncompress = true;
    }
    Config::Bind(Utf8izeReplace, ini, config, "Server.Utf8izeReplace", true);

    Config::Bind(StartupDocument, ini, config, "Server.StartupDocument");
    normalizePath(StartupDocument);
    Config::Bind(RequestInitFunction, ini, config,
                 "Server.RequestInitFunction");
    Config::Bind(RequestInitDocument, ini, config,
                 "Server.RequestInitDocument");
    Config::Bind(SafeFileAccess, ini, config, "Server.SafeFileAccess");
    Config::Bind(AllowedDirectories, ini, config, "Server.AllowedDirectories");
    Config::Bind(WhitelistExec, ini, config, "Server.WhitelistExec");
    Config::Bind(WhitelistExecWarningOnly, ini, config,
                 "Server.WhitelistExecWarningOnly");
    Config::Bind(AllowedExecCmds, ini, config, "Server.AllowedExecCmds");
    Config::Bind(UnserializationWhitelistCheck, ini, config,
                 "Server.UnserializationWhitelistCheck", false);
    Config::Bind(UnserializationWhitelistCheckWarningOnly, ini, config,
                 "Server.UnserializationWhitelistCheckWarningOnly", true);
    Config::Bind(AllowedFiles, ini, config, "Server.AllowedFiles");
    Config::Bind(ForbiddenFileExtensions, ini, config,
                 "Server.ForbiddenFileExtensions");
    Config::Bind(LockCodeMemory, ini, config, "Server.LockCodeMemory", false);
    Config::Bind(MaxArrayChain, ini, config, "Server.MaxArrayChain", INT_MAX);
    if (MaxArrayChain != INT_MAX) {
      // MixedArray needs a higher threshold to avoid false-positives.
      // (and we always use MixedArray)
      MaxArrayChain *= 2;
    }

    Config::Bind(WarnOnCollectionToArray, ini, config,
                 "Server.WarnOnCollectionToArray", false);
    Config::Bind(UseDirectCopy, ini, config, "Server.UseDirectCopy", false);
    Config::Bind(AlwaysUseRelativePath, ini, config,
                 "Server.AlwaysUseRelativePath", false);
    {
      // Server Upload
      Config::Bind(UploadMaxFileSize, ini, config,
                   "Server.Upload.UploadMaxFileSize", 100);
      UploadMaxFileSize <<= 20;
      Config::Bind(UploadTmpDir, ini, config, "Server.Upload.UploadTmpDir",
                   "/tmp");
      Config::Bind(EnableFileUploads, ini, config,
                   "Server.Upload.EnableFileUploads", true);
      Config::Bind(MaxFileUploads, ini, config, "Server.Upload.MaxFileUploads",
                   20);
      Config::Bind(EnableUploadProgress, ini, config,
                   "Server.Upload.EnableUploadProgress");
      Config::Bind(Rfc1867Freq, ini, config, "Server.Upload.Rfc1867Freq",
                   256 * 1024);
      if (Rfc1867Freq < 0) Rfc1867Freq = 256 * 1024;
      Config::Bind(Rfc1867Prefix, ini, config, "Server.Upload.Rfc1867Prefix",
                   "vupload_");
      Config::Bind(Rfc1867Name, ini, config, "Server.Upload.Rfc1867Name",
                   "video_ptoken");
    }
    Config::Bind(ImageMemoryMaxBytes, ini, config,
                 "Server.ImageMemoryMaxBytes", 0);
    if (ImageMemoryMaxBytes == 0) {
      ImageMemoryMaxBytes = UploadMaxFileSize * 2;
    }
    Config::Bind(LightProcessFilePrefix, ini, config,
                 "Server.LightProcessFilePrefix", "./lightprocess");
    Config::Bind(LightProcessCount, ini, config,
                 "Server.LightProcessCount", 0);
    Config::Bind(ForceServerNameToHeader, ini, config,
                 "Server.ForceServerNameToHeader");
    Config::Bind(AllowDuplicateCookies, ini, config,
                 "Server.AllowDuplicateCookies", !EnableHipHopSyntax);
    Config::Bind(EnableCufAsync, ini, config, "Server.EnableCufAsync",
                 false);
    Config::Bind(PathDebug, ini, config, "Server.PathDebug", false);
    Config::Bind(ServerUser, ini, config, "Server.User", "");
  }

  VirtualHost::SortAllowedDirectories(AllowedDirectories);
  {
    if (config["VirtualHost"].exists()) {
      for (Hdf hdf = config["VirtualHost"].firstChild(); hdf.exists();
           hdf = hdf.next()) {
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
    // IpBlocks
    IpBlocks = std::make_shared<IpBlockMap>(ini, config["IpBlockMap"]);
  }
  {
    if (config["Satellites"].exists()) {
      for (Hdf hdf = config["Satellites"].firstChild(); hdf.exists();
           hdf = hdf.next()) {
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
    // Xbox
    Config::Bind(XboxServerThreadCount, ini, config,
                 "Xbox.ServerInfo.ThreadCount", 10);
    Config::Bind(XboxServerMaxQueueLength, ini, config,
                 "Xbox.ServerInfo.MaxQueueLength", INT_MAX);
    if (XboxServerMaxQueueLength < 0) XboxServerMaxQueueLength = INT_MAX;
    Config::Bind(XboxServerPort, ini, config, "Xbox.ServerInfo.Port", 0);
    Config::Bind(XboxDefaultLocalTimeoutMilliSeconds, ini, config,
                 "Xbox.DefaultLocalTimeoutMilliSeconds", 500);
    Config::Bind(XboxDefaultRemoteTimeoutSeconds, ini, config,
                 "Xbox.DefaultRemoteTimeoutSeconds", 5);
    Config::Bind(XboxServerInfoMaxRequest, ini, config,
                 "Xbox.ServerInfo.MaxRequest", 500);
    Config::Bind(XboxServerInfoDuration, ini, config,
                 "Xbox.ServerInfo.MaxDuration", 120);
    Config::Bind(XboxServerInfoWarmupDoc, ini, config,
                 "Xbox.ServerInfo.WarmupDocument", "");
    Config::Bind(XboxServerInfoReqInitFunc, ini, config,
                 "Xbox.ServerInfo.RequestInitFunction", "");
    Config::Bind(XboxServerInfoReqInitDoc, ini, config,
                 "Xbox.ServerInfo.RequestInitDocument", "");
    Config::Bind(XboxServerInfoAlwaysReset, ini, config,
                 "Xbox.ServerInfo.AlwaysReset", false);
    Config::Bind(XboxServerLogInfo, ini, config, "Xbox.ServerInfo.LogInfo",
                 false);
    Config::Bind(XboxProcessMessageFunc, ini, config, "Xbox.ProcessMessageFunc",
                 "xbox_process_message");
  }
  {
    // Pagelet Server
    Config::Bind(PageletServerThreadCount, ini, config,
                 "PageletServer.ThreadCount", 0);
    Config::Bind(PageletServerThreadRoundRobin, ini, config,
                 "PageletServer.ThreadRoundRobin");
    Config::Bind(PageletServerThreadDropStack, ini, config,
                 "PageletServer.ThreadDropStack");
    Config::Bind(PageletServerThreadDropCacheTimeoutSeconds, ini, config,
                 "PageletServer.ThreadDropCacheTimeoutSeconds", 0);
    Config::Bind(PageletServerQueueLimit, ini, config,
                 "PageletServer.QueueLimit", 0);
  }
  {
    // Static File
    Config::Bind(StaticFileExtensions, ini, config, "StaticFile.Extensions");
    Config::Bind(StaticFileGenerators, ini, config, "StaticFile.Generators");

    auto matches_callback = [] (const IniSettingMap &ini_m, const Hdf &hdf_m,
                                const std::string &ini_m_key) {
      FilesMatches.push_back(std::make_shared<FilesMatch>(ini_m, hdf_m));
    };
    Config::Iterate(matches_callback, ini, config, "StaticFile.FilesMatch");
  }
  {
    // PhpFile
    Config::Bind(PhpFileExtensions, ini, config, "PhpFile.Extensions");
  }
  {
    // Admin Server
    Config::Bind(AdminServerPort, ini, config, "AdminServer.Port", 0);
    Config::Bind(AdminThreadCount, ini, config, "AdminServer.ThreadCount", 1);
    Config::Bind(AdminPassword, ini, config, "AdminServer.Password");
    Config::Bind(AdminPasswords, ini, config, "AdminServer.Passwords");
  }
  {
    // Proxy
    Config::Bind(ProxyOriginRaw, ini, config, "Proxy.Origin");
    Config::Bind(ProxyPercentageRaw, ini, config, "Proxy.Percentage", 0);
    Config::Bind(ProxyRetry, ini, config, "Proxy.Retry", 3);
    Config::Bind(UseServeURLs, ini, config, "Proxy.ServeURLs");
    Config::Bind(ServeURLs, ini, config, "Proxy.ServeURLs");
    Config::Bind(UseProxyURLs, ini, config, "Proxy.ProxyURLs");
    Config::Bind(ProxyURLs, ini, config, "Proxy.ProxyURLs");
    Config::Bind(ProxyPatterns, ini, config, "Proxy.ProxyPatterns");
  }
  {
    // Http
    Config::Bind(HttpDefaultTimeout, ini, config, "Http.DefaultTimeout", 30);
    Config::Bind(HttpSlowQueryThreshold, ini, config, "Http.SlowQueryThreshold",
                 5000);
  }
  {
    // Debug

    Config::Bind(NativeStackTrace, ini, config, "Debug.NativeStackTrace");
    StackTrace::Enabled = NativeStackTrace;
    Config::Bind(TranslateLeakStackTrace, ini, config,
                 "Debug.TranslateLeakStackTrace");
    Config::Bind(FullBacktrace, ini, config, "Debug.FullBacktrace");
    Config::Bind(ServerErrorMessage, ini, config, "Debug.ServerErrorMessage");
    Config::Bind(TranslateSource, ini, config, "Debug.TranslateSource");
    Config::Bind(RecordInput, ini, config, "Debug.RecordInput");
    Config::Bind(ClearInputOnSuccess, ini, config, "Debug.ClearInputOnSuccess",
                 true);
    Config::Bind(ProfilerOutputDir, ini, config, "Debug.ProfilerOutputDir",
                 "/tmp");
    Config::Bind(CoreDumpEmail, ini, config, "Debug.CoreDumpEmail");
    Config::Bind(CoreDumpReport, ini, config, "Debug.CoreDumpReport", true);
    if (CoreDumpReport) {
      install_crash_reporter();
    }

    auto core_dump_report_dir =
      Config::Get(ini, config, "Debug.CoreDumpReportDirectory",
#if defined(HPHP_OSS)
  "/tmp"
#else
  "/var/tmp/cores"
#endif
      );
    std::ostringstream stack_trace_stream;
    stack_trace_stream << core_dump_report_dir << "/stacktrace."
                       << Process::GetProcessId() << ".log";
    StackTraceFilename = stack_trace_stream.str();

    Config::Bind(LocalMemcache, ini, config, "Debug.LocalMemcache");
    Config::Bind(MemcacheReadOnly, ini, config, "Debug.MemcacheReadOnly");
    Config::Bind(StackTraceTimeout, ini, config, "Debug.StackTraceTimeout", 0);

    {
      // Debug SimpleCounter
      Config::Bind(SimpleCounter::SampleStackCount, ini,
                   config, "Debug.SimpleCounter.SampleStackCount", 0);
      Config::Bind(SimpleCounter::SampleStackDepth, ini,
                   config, "Debug.SimpleCounter.SampleStackDepth", 5);
    }
  }
  {
    // Stats
    Config::Bind(EnableStats, ini, config, "Stats.Enable",
                 false); // main switch
    Config::Bind(EnableAPCStats, ini, config, "Stats.APC", false);
    Config::Bind(EnableWebStats, ini, config, "Stats.Web");
    Config::Bind(EnableMemoryStats, ini, config, "Stats.Memory");
    Config::Bind(EnableSQLStats, ini, config, "Stats.SQL");
    Config::Bind(EnableSQLTableStats, ini, config, "Stats.SQLTable");
    Config::Bind(EnableNetworkIOStatus, ini, config, "Stats.NetworkIO");
    Config::Bind(StatsXSL, ini, config, "Stats.XSL");
    Config::Bind(StatsXSLProxy, ini, config, "Stats.XSLProxy");
    Config::Bind(StatsSlotDuration, ini, config, "Stats.SlotDuration", 10 * 60);
    Config::Bind(StatsMaxSlot, ini, config, "Stats.MaxSlot",
                 12 * 6); // 12 hours
    Config::Bind(EnableHotProfiler, ini, config, "Stats.EnableHotProfiler",
                 true);
    Config::Bind(ProfilerTraceBuffer, ini, config, "Stats.ProfilerTraceBuffer",
                 2000000);
    Config::Bind(ProfilerTraceExpansion, ini, config,
                 "Stats.ProfilerTraceExpansion", 1.2);
    Config::Bind(ProfilerMaxTraceBuffer, ini, config,
                 "Stats.ProfilerMaxTraceBuffer", 0);
  }
  {
    Config::Bind(ServerVariables, ini, config, "ServerVariables");
    Config::Bind(EnvVariables, ini, config, "EnvVariables");
  }
  {
    // Sandbox
    Config::Bind(SandboxMode, ini, config, "Sandbox.SandboxMode");
    SandboxPattern = format_pattern(Config::GetString(ini, config,
                                                      "Sandbox.Pattern"),
                                    true);
    Config::Bind(SandboxHome, ini, config, "Sandbox.Home");
    Config::Bind(SandboxFallback, ini, config, "Sandbox.Fallback");
    Config::Bind(SandboxConfFile, ini, config, "Sandbox.ConfFile");
    Config::Bind(SandboxFromCommonRoot, ini, config, "Sandbox.FromCommonRoot");
    Config::Bind(SandboxDirectoriesRoot, ini, config,
                 "Sandbox.DirectoriesRoot");
    Config::Bind(SandboxLogsRoot, ini, config, "Sandbox.LogsRoot");
    Config::Bind(SandboxServerVariables, ini, config,
                 "Sandbox.ServerVariables");
  }
  {
    // Mail
    Config::Bind(SendmailPath, ini, config, "Mail.SendmailPath",
                 "/usr/lib/sendmail -t -i");
    Config::Bind(MailForceExtraParameters, ini, config,
                 "Mail.ForceExtraParameters");
  }
  {
    // Preg
    Config::Bind(PregBacktraceLimit, ini, config, "Preg.BacktraceLimit",
                 1000000);
    Config::Bind(PregRecursionLimit, ini, config, "Preg.RecursionLimit",
                 100000);
    Config::Bind(EnablePregErrorLog, ini, config, "Preg.ErrorLog", true);
  }
  {
    Config::Bind(HHProfServerEnabled, ini, config, "HHProfServer.Enabled",
                 false);
    Config::Bind(HHProfServerPort, ini, config, "HHProfServer.Port", 4327);
    Config::Bind(HHProfServerThreads, ini, config, "HHProfServer.Threads", 2);
    Config::Bind(HHProfServerTimeoutSeconds, ini, config,
                 "HHProfServer.TimeoutSeconds", 30);
    Config::Bind(HHProfServerProfileClientMode, ini, config,
                 "HHProfServer.ProfileClientMode", true);
    Config::Bind(HHProfServerAllocationProfile, ini, config,
                 "HHProfServer.AllocationProfile", false);
    {
      // HHProfServer.Filter.*
      Config::Bind(HHProfServerFilterMinAllocPerReq, ini, config,
                   "HHProfServer.Filter.MinAllocPerReq", 2);
      Config::Bind(HHProfServerFilterMinBytesPerReq, ini, config,
                   "HHProfServer.Filter.MinBytesPerReq", 128);
    }
  }
  {
    // SimpleXML
    Config::Bind(SimpleXMLEmptyNamespaceMatchesAll, ini, config,
                 "SimpleXML.EmptyNamespaceMatchesAll", false);
  }
#ifdef FACEBOOK
  {
    // Fb303Server
    Config::Bind(EnableFb303Server, ini, config, "Fb303Server.Enable",
                 EnableFb303Server);
    Config::Bind(Fb303ServerPort, ini, config, "Fb303Server.Port", 0);
    Config::Bind(Fb303ServerThreadStackSizeMb, ini, config,
                 "Fb303Server.ThreadStackSizeMb", 8);
    Config::Bind(Fb303ServerWorkerThreads, ini, config,
                 "Fb303Server.WorkerThreads", 1);
    Config::Bind(Fb303ServerPoolThreads, ini, config, "Fb303Server.PoolThreads",
                 1);
  }
#endif

  {
    // Xenon
    Config::Bind(XenonPeriodSeconds, ini, config, "Xenon.Period", 0.0);
    Config::Bind(XenonForceAlwaysOn, ini, config, "Xenon.ForceAlwaysOn", false);
  }

  Config::Bind(CustomSettings, ini, config, "CustomSettings");

  refineStaticStringTableSize();

  // Enables the hotfixing of a bug that occurred with D1797805 where
  // per request user settings (like upload_max_filesize) were not able to be
  // accessed on a server request any longer. The longer term fix is in review
  // D2099778, but we want that to simmer in Master for a while and we need
  // a hotfix for our current 3.6 LTS (Github Issue #4993)
  Config::Bind(EnableZendIniCompat, ini, config, "Eval.EnableZendIniCompat",
               true);
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
  IniSetting::Bind(IniSetting::CORE, IniSetting::PHP_INI_SYSTEM,
                   "sendmail_path", &RuntimeOption::SendmailPath);

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
  IniSetting::Bind(IniSetting::CORE, IniSetting::PHP_INI_ONLY,
                   "hhvm.build_type",
                   IniSetting::SetAndGet<std::string>(
    [](const std::string&) {
      return false;
    },
    []() {
      return s_hhvm_build_type.c_str();
    }
  ));

  // Extensions
  Config::Bind(RuntimeOption::ExtensionDir, ini, config, "extension_dir",
               RuntimeOption::ExtensionDir, false);
  Config::Bind(RuntimeOption::DynamicExtensionPath, ini,
               config, "DynamicExtensionPath",
               RuntimeOption::DynamicExtensionPath);
  Config::Bind(RuntimeOption::Extensions, ini, config, "extensions");
  Config::Bind(RuntimeOption::DynamicExtensions, ini,
               config, "DynamicExtensions");


  ExtensionRegistry::moduleLoad(ini, config);
  extern void initialize_apc();
  initialize_apc();
}

///////////////////////////////////////////////////////////////////////////////
}
