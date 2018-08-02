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

#ifndef incl_HPHP_RUNTIME_OPTION_H_
#define incl_HPHP_RUNTIME_OPTION_H_

#include <folly/dynamic.h>

#include <unordered_map>
#include <vector>
#include <string>
#include <map>
#include <set>
#include <boost/container/flat_set.hpp>
#include <memory>

#include "hphp/runtime/base/config.h"
#include "hphp/runtime/base/typed-value.h"
#include "hphp/util/compilation-flags.h"
#include "hphp/util/hash-map.h"
#include "hphp/util/functional.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

struct AccessLogFileData;
struct ErrorLogFileData;
struct VirtualHost;
struct IpBlockMap;
struct SatelliteServerInfo;
struct FilesMatch;
struct Hdf;
struct IniSettingMap;

constexpr int kDefaultInitialStaticStringTableSize = 500000;

enum class JitSerdesMode {
  Off,
  Serialize,
  SerializeAndExit,
  Deserialize,
  DeserializeOrFail,
  DeserializeOrGenerate,
  DeserializeAndExit,
};

/**
 * Configurable options set from command line or configurable file at startup
 * time.
 */
struct RuntimeOption {
  static void Load(
    IniSettingMap &ini, Hdf& config,
    const std::vector<std::string>& iniClis = std::vector<std::string>(),
    const std::vector<std::string>& hdfClis = std::vector<std::string>(),
    std::vector<std::string>* messages = nullptr,
    std::string cmd = "");

  static bool ServerExecutionMode() {
    return ServerMode;
  }

  static bool GcSamplingEnabled() {
    return EvalGCSampleRate > 0;
  }

  static bool JitSamplingEnabled() {
    return EvalJit && EvalJitSampleRate > 0;
  }

  static bool AllowObjectDestructors() {
    // When one_bit_refcount == true, skip the runtime check since destructors
    // are never allowed.
    return !one_bit_refcount && EvalAllowObjectDestructors;
  }

  static void ReadSatelliteInfo(
    const IniSettingMap& ini,
    const Hdf& hdf,
    std::vector<std::shared_ptr<SatelliteServerInfo>>& infos,
    std::string& xboxPassword,
    std::set<std::string>& xboxPasswords
  );

  static std::string getTraceOutputFile();

  static bool ServerMode;
  static std::string BuildId;
  static std::string InstanceId;
  static std::string DeploymentId; // ID for set of instances deployed at once
  static std::string PidFile;

  static std::map<std::string, ErrorLogFileData> ErrorLogs;
  static std::string LogFile;
  static std::string LogFileSymLink;
  static uint16_t LogFilePeriodMultiplier;

  static int LogHeaderMangle;
  static bool AlwaysEscapeLog;
  static bool AlwaysLogUnhandledExceptions;
  static bool NoSilencer;
  static int ErrorUpgradeLevel; // Bitmask of errors to upgrade to E_USER_ERROR
  static bool CallUserHandlerOnFatals;
  static bool ThrowExceptionOnBadMethodCall;
  static bool LogNativeStackOnOOM;
  static int RuntimeErrorReportingLevel;
  static int ForceErrorReportingLevel; // Bitmask ORed with the reporting level

  static std::string ServerUser; // run server under this user account
  static bool AllowRunAsRoot; // Allow running hhvm as root.

  static int  MaxSerializedStringSize;
  static bool NoInfiniteRecursionDetection;
  static bool AssertEmitted;
  static int64_t NoticeFrequency; // output 1 out of NoticeFrequency notices
  static int64_t WarningFrequency;
  static int RaiseDebuggingFrequency;
  static int64_t SerializationSizeLimit;

  static std::string AccessLogDefaultFormat;
  static std::map<std::string, AccessLogFileData> AccessLogs;

  static std::string AdminLogFormat;
  static std::string AdminLogFile;
  static std::string AdminLogSymLink;

  static std::map<std::string, AccessLogFileData> RPCLogs;

  static std::string Host;
  static std::string DefaultServerNameSuffix;
  static std::string ServerType;
  static std::string ServerIP;
  static std::string ServerFileSocket;
  static const std::string& GetServerPrimaryIPv4();
  static const std::string& GetServerPrimaryIPv6();
  static int ServerPort;
  static int ServerPortFd;
  static int ServerBacklog;
  static int ServerConnectionLimit;
  static int ServerThreadCount;
  static int ServerQueueCount;
  // Number of worker threads with stack partially on huge pages.
  static int ServerHugeThreadCount;
  static int ServerHugeStackKb;
  static int ServerWarmupThrottleRequestCount;
  static int ServerThreadDropCacheTimeoutSeconds;
  static int ServerThreadJobLIFOSwitchThreshold;
  static int ServerThreadJobMaxQueuingMilliSeconds;
  static bool AlwaysDecodePostDataDefault;
  static bool ServerThreadDropStack;
  static bool ServerHttpSafeMode;
  static bool ServerStatCache;
  static bool ServerFixPathInfo;
  static bool ServerAddVaryEncoding;
  static bool ServerLogSettingsOnStartup;
  static bool ServerForkEnabled;
  static bool ServerForkLogging;
  static std::vector<std::string> ServerWarmupRequests;
  static std::string ServerCleanupRequest;
  static int ServerInternalWarmupThreads;
  static boost::container::flat_set<std::string> ServerHighPriorityEndPoints;
  static bool ServerExitOnBindFail;
  static int PageletServerThreadCount;
  static int PageletServerHugeThreadCount;
  static int PageletServerThreadDropCacheTimeoutSeconds;
  static int PageletServerQueueLimit;
  static bool PageletServerThreadDropStack;

  static int RequestTimeoutSeconds;
  static int PspTimeoutSeconds;
  static int PspCpuTimeoutSeconds;
  static int64_t MaxRequestAgeFactor;
  static int64_t RequestMemoryMaxBytes;
  // Threshold for aborting when host is low on memory.
  static int64_t RequestMemoryOOMKillBytes;
  // Approximate upper bound for thread heap that is backed by huge pages.  This
  // doesn't include the first slab colocated with thread stack, if any.
  static int64_t RequestHugeMaxBytes;
  static int64_t ImageMemoryMaxBytes;
  static int ServerGracefulShutdownWait;
  static bool ServerHarshShutdown;
  static bool ServerEvilShutdown;
  static bool ServerKillOnSIGTERM;
  static bool ServerKillOnTimeout;
  static int ServerPreShutdownWait;
  static int ServerShutdownListenWait;
  static int ServerShutdownEOMWait;
  static int ServerPrepareToStopTimeout;
  static int ServerPartialPostStatusCode;
  // If `StopOldServer` is set, we try to stop the old server running
  // on the local host earlier when we initialize, and we do not start
  // serving requests until we are confident that the system can give
  // the new server `ServerRSSNeededMb` resident memory, or till
  // `OldServerWait` seconds passes after an effort to stop the old
  // server is made.
  static bool StopOldServer;
  static int64_t ServerRSSNeededMb;
  // Threshold of free memory below which the old server is shutdown immediately
  // upon a memory pressure check.
  static int64_t ServerCriticalFreeMb;
  static int OldServerWait;
  // The percentage of page caches that can be considered as free (0 -
  // 100).  This is experimental.
  static int CacheFreeFactor;
  static std::vector<std::string> ServerNextProtocols;
  static bool ServerEnableH2C;
  static int BrotliCompressionEnabled;
  static int BrotliChunkedCompressionEnabled;
  static int BrotliCompressionMode;
  // Base 2 logarithm of the sliding window size. Range is 10-24.
  static int BrotliCompressionLgWindowSize;
  static int BrotliCompressionQuality;
  static int GzipCompressionLevel;
  static int GzipMaxCompressionLevel;
  static std::string ForceCompressionURL;
  static std::string ForceCompressionCookie;
  static std::string ForceCompressionParam;
  static bool EnableKeepAlive;
  static bool ExposeHPHP;
  static bool ExposeXFBServer;
  static bool ExposeXFBDebug;
  static std::string XFBDebugSSLKey;
  static int ConnectionTimeoutSeconds;
  static bool EnableOutputBuffering;
  static std::string OutputHandler;
  static bool ImplicitFlush;
  static bool EnableEarlyFlush;
  static bool ForceChunkedEncoding;
  static int64_t MaxPostSize;
  static int64_t LowestMaxPostSize;
  static bool AlwaysPopulateRawPostData;
  static int64_t UploadMaxFileSize;
  static std::string UploadTmpDir;
  static bool EnableFileUploads;
  static bool EnableUploadProgress;
  static int64_t MaxFileUploads;
  static int Rfc1867Freq;
  static std::string Rfc1867Prefix;
  static std::string Rfc1867Name;
  static bool ExpiresActive;
  static int ExpiresDefault;
  static std::string DefaultCharsetName;
  static bool ForceServerNameToHeader;
  static bool PathDebug;
  static std::vector<std::shared_ptr<VirtualHost>> VirtualHosts;
  static std::shared_ptr<IpBlockMap> IpBlocks;
  static std::vector<std::shared_ptr<SatelliteServerInfo>>
         SatelliteServerInfos;

  // If a request has a body over this limit, switch to on-demand reading.
  // -1 for no limit.
  static int64_t RequestBodyReadLimit;

  static bool EnableSSL;
  static int SSLPort;
  static int SSLPortFd;
  static std::string SSLCertificateFile;
  static std::string SSLCertificateKeyFile;
  static std::string SSLCertificateDir;
  static std::string SSLTicketSeedFile;
  static bool TLSDisableTLS1_2;
  static std::string TLSClientCipherSpec;
  static bool EnableSSLWithPlainText;

  static int XboxServerThreadCount;
  static int XboxServerMaxQueueLength;
  static int XboxServerPort;
  static int XboxDefaultLocalTimeoutMilliSeconds;
  static int XboxDefaultRemoteTimeoutSeconds;
  static int XboxServerInfoMaxRequest;
  static int XboxServerInfoDuration;
  static std::string XboxServerInfoWarmupDoc;
  static std::string XboxServerInfoReqInitFunc;
  static std::string XboxServerInfoReqInitDoc;
  static bool XboxServerInfoAlwaysReset;
  static bool XboxServerLogInfo;
  static std::string XboxProcessMessageFunc;
  static std::string XboxPassword;
  static std::set<std::string> XboxPasswords;

  static std::string SourceRoot;
  static std::vector<std::string> IncludeSearchPaths;

  /**
   * Legal root directory expressions in an include expression. For example,
   *
   *   include_once $PHP_ROOT . '/lib.php';
   *
   * Here, "$PHP_ROOT" is a legal include root. Stores what it resolves to.
   *
   *   RuntimeOption::IncludeRoots["$PHP_ROOT"] = "";
   *   RuntimeOption::IncludeRoots["$LIB_ROOT"] = "lib";
   */
  static std::map<std::string, std::string> IncludeRoots;
  static std::map<std::string, std::string> AutoloadRoots;

  static std::string FileCache;
  static std::string DefaultDocument;
  static std::string ErrorDocument404;
  static bool ForbiddenAs404;
  static std::string ErrorDocument500;
  static std::string FatalErrorMessage;
  static std::string FontPath;
  static bool EnableStaticContentFromDisk;
  static bool EnableOnDemandUncompress;
  static bool EnableStaticContentMMap;

  static bool Utf8izeReplace;

  static std::string RequestInitFunction;
  static std::string RequestInitDocument;
  static std::string AutoPrependFile;
  static std::string AutoAppendFile;

  static bool SafeFileAccess;
  static std::vector<std::string> AllowedDirectories;
  static std::set<std::string> AllowedFiles;
  static hphp_string_imap<std::string> StaticFileExtensions;
  static hphp_string_imap<std::string> PhpFileExtensions;
  static std::set<std::string> ForbiddenFileExtensions;
  static std::set<std::string> StaticFileGenerators;
  static std::vector<std::shared_ptr<FilesMatch>> FilesMatches;

  static bool WhitelistExec;
  static bool WhitelistExecWarningOnly;
  static std::vector<std::string> AllowedExecCmds;

  static bool UnserializationWhitelistCheck;
  static bool UnserializationWhitelistCheckWarningOnly;
  static int64_t UnserializationBigMapThreshold;

  static std::string TakeoverFilename;
  static std::string AdminServerIP;
  static int AdminServerPort;
  static int AdminThreadCount;
  static std::string AdminPassword;
  static std::set<std::string> AdminPasswords;
  static std::set<std::string> HashedAdminPasswords;

  /*
   * Options related to reverse proxying. ProxyOriginRaw and ProxyPercentageRaw
   * may be mutated by background threads and should only be read or written
   * using the helper functions defined with HttpRequestHandler.
   */
  static std::string ProxyOriginRaw;
  static int ProxyPercentageRaw;
  static int ProxyRetry;
  static bool UseServeURLs;
  static std::set<std::string> ServeURLs;
  static bool UseProxyURLs;
  static std::set<std::string> ProxyURLs;
  static std::vector<std::string> ProxyPatterns;
  static bool AlwaysUseRelativePath;

  static int  HttpDefaultTimeout;
  static int  HttpSlowQueryThreshold;

  static bool NativeStackTrace;
  static bool ServerErrorMessage;
  static bool RecordInput;
  static bool ClearInputOnSuccess;
  static std::string ProfilerOutputDir;
  static std::string CoreDumpEmail;
  static bool CoreDumpReport;
  static std::string CoreDumpReportDirectory;
  static std::string StackTraceFilename;
  static int StackTraceTimeout;
  static std::string RemoteTraceOutputDir;
  static std::set<std::string, stdltistr> TraceFunctions;
  static uint32_t TraceFuncId;

  static bool EnableStats;
  static bool EnableAPCStats;
  static bool EnableWebStats;
  static bool EnableMemoryStats;
  static bool EnableSQLStats;
  static bool EnableSQLTableStats;
  static bool EnableNetworkIOStatus;
  static std::string StatsXSL;
  static std::string StatsXSLProxy;
  static int StatsSlotDuration;
  static int StatsMaxSlot;

  static bool EnableHotProfiler;
  static int32_t ProfilerTraceBuffer;
  static double ProfilerTraceExpansion;
  static int32_t ProfilerMaxTraceBuffer;

  static int64_t MaxSQLRowCount;
  static int64_t SocketDefaultTimeout;
  static bool LockCodeMemory;
  static int MaxArrayChain;
  static bool WarnOnCollectionToArray;
  static bool UseDirectCopy;

  static bool DisableSmallAllocator;

  static int PerAllocSampleF;
  static int TotalAllocSampleF;

  static std::map<std::string, std::string> ServerVariables;

  static std::map<std::string, std::string> EnvVariables;

  // The file name that is used by LightProcess to bind the socket
  // is the following prefix followed by the pid of the hphp process.
  static std::string LightProcessFilePrefix;
  static int LightProcessCount;

  // Eval options
  static bool EnableHipHopSyntax;
  static bool EnableHipHopExperimentalSyntax;
  static bool EnableShortTags;
  static bool EnableAspTags;
  static bool EnableXHP;
  static bool CheckParamTypeInvariance;
  static bool EnableObjDestructCall;
  static bool EnableIntrinsicsExtension;
  static bool CheckSymLink;
  static bool EnableArgsInBacktraces;
  static bool EnableContextInErrorHandler;
  static bool EnableZendSorting;
  static bool EnableZendIniCompat;
  static bool TimeoutsUseWallTime;
  static bool CheckFlushOnUserClose;
  static bool EvalAuthoritativeMode;
  static bool IntsOverflowToInts;
  static HackStrictOption StrictArrayFillKeys;
  static HackStrictOption DisallowDynamicVarEnvFuncs;
  static HackStrictOption IconvIgnoreCorrect;
  static HackStrictOption MinMaxAllowDegenerate;
  static bool LookForTypechecker;
  static bool AutoTypecheck;
  static bool AutoprimeGenerators;
  static bool EnableIsExprPrimitiveMigration;
  static bool EnableCoroutines;
  static bool Hacksperimental;
  static uint32_t EvalInitialStaticStringTableSize;
  static uint32_t EvalInitialNamedEntityTableSize;
  static JitSerdesMode EvalJitSerdesMode;
  static std::string EvalJitSerdesFile;
  static bool DumpPreciseProfileData;

  // ENABLED (1) selects PHP7 behavior.
  static bool PHP7_DeprecationWarnings;
  static bool PHP7_IntSemantics;
  static bool PHP7_LTR_assign;
  static bool PHP7_NoHexNumerics;
  static bool PHP7_Builtins;
  static bool PHP7_ScalarTypes;
  static bool PHP7_EngineExceptions;
  static bool PHP7_Substr;
  static bool PHP7_UVS;
  static bool PHP7_DisallowUnsafeCurlUploads;

  static int64_t HeapSizeMB;
  static int64_t HeapResetCountBase;
  static int64_t HeapResetCountMultiple;
  static int64_t HeapLowWaterMark;
  static int64_t HeapHighWaterMark;

  static int GetScannerType();

  static std::set<std::string, stdltistr> DynamicInvokeFunctions;
  static hphp_string_imap<Cell> ConstantFunctions;

  static const uint32_t kPCREInitialTableSize = 96 * 1024;

  static std::string ExtensionDir;
  static std::vector<std::string> Extensions;
  static std::string DynamicExtensionPath;
  static std::vector<std::string> DynamicExtensions;

  // Namespace aliases for the compiler
  static std::map<std::string, std::string> AliasedNamespaces;

  static std::vector<std::string> TzdataSearchPaths;

#define EVALFLAGS()                                                     \
  /* F(type, name, defaultVal) */                                       \
  /*                                                                    \
   * Maximum number of elements on the VM execution stack.              \
   */                                                                   \
  F(uint64_t, VMStackElms, kEvalVMStackElmsDefault)                     \
  /*                                                                    \
   * Initial space reserved for the global variable environment (in     \
   * number of global variables).                                       \
   */                                                                   \
  F(uint32_t, VMInitialGlobalTableSize,                                 \
    kEvalVMInitialGlobalTableSizeDefault)                               \
  F(bool, Jit,                         evalJitDefault())                \
  F(bool, JitEvaledCode,               true)                            \
  F(bool, JitRequireWriteLease,        false)                           \
  F(uint64_t, JitRelocationSize,       kJitRelocationSizeDefault)       \
  F(uint64_t, JitMatureSize,           125 << 20)                       \
  F(bool, JitMatureAfterWarmup,        true)                            \
  F(double, JitMaturityExponent,       1.)                              \
  F(bool, JitTimer,                    kJitTimerDefault)                \
  F(int, JitConcurrently,              1)                               \
  F(int, JitThreads,                   4)                               \
  F(int, JitWorkerThreads,             Process::GetCPUCount() / 2)      \
  F(int, JitWorkerThreadsForSerdes,    0)                               \
  F(bool, JitDesProfDataAfterRetranslateAll, true)                      \
  F(int, JitLdimmqSpan,                8)                               \
  F(int, JitPrintOptimizedIR,          0)                               \
  F(bool, RecordSubprocessTimes,       false)                           \
  F(bool, AllowHhas,                   false)                           \
  F(bool, DisassemblerSourceMapping,   true)                            \
  F(bool, DisableReturnByReference,    false)                           \
  F(bool, GenerateDocComments,         true)                            \
  F(bool, DisassemblerDocComments,     true)                            \
  F(bool, DisassemblerPropDocComments, true)                            \
  F(bool, LoadFilepathFromUnitCache,   false)                           \
  F(bool, ThrowOnCallByRefAnnotationMismatch, false)                    \
  F(bool, WarnOnCallByRefAnnotationMismatch, true)                      \
  F(bool, WarnOnCoerceBuiltinParams, false)                             \
  /* Whether to use the embedded hackc binary */                        \
  F(bool, HackCompilerUseEmbedded,     facebook)                        \
  /* Whether to trust existing versions of the extracted compiler */    \
  F(bool, HackCompilerTrustExtract,    true)                            \
  /* When using an embedded hackc, extract it to the ExtractPath or the
     ExtractFallback. */                                                \
  F(string, HackCompilerExtractPath,   "/var/run/hackc_%{schema}")      \
  F(string, HackCompilerFallbackPath,  "/tmp/hackc_%{schema}_XXXXXX")   \
  /* Arguments to run embedded hackc binary with */                     \
  F(string, HackCompilerArgs,          hackCompilerArgsDefault())       \
  /* The command to invoke to spawn hh_single_compile in server mode. */\
  F(string, HackCompilerCommand,       hackCompilerCommandDefault())    \
  /* The number of hh_single_compile daemons to keep alive. */          \
  F(uint64_t, HackCompilerWorkers,     Process::GetCPUCount())      \
  /* The number of times to retry after an infra failure communicating
     with a compiler process. */                                        \
  F(uint64_t, HackCompilerMaxRetries,  0)                               \
  /* Whether to log extern compiler performance */                      \
  F(bool, LogExternCompilerPerf,       false)                           \
  /* Whether to write verbose log messages to the error log and include
     the hhas from failing units in the fatal error messages produced by
     bad hh_single_compile units. */                                    \
  F(bool, HackCompilerVerboseErrors,   true)                            \
  /* Whether the HackC compiler should inherit the compiler config of the
     HHVM process that launches it. */                                  \
  F(bool, HackCompilerInheritConfig,   true)                            \
  /* When using embedded data, extract it to the ExtractPath or the
   * ExtractFallback. */                                                \
  F(string, EmbeddedDataExtractPath,   "/var/run/hhvm_%{type}_%{buildid}") \
  F(string, EmbeddedDataFallbackPath,  "/tmp/hhvm_%{type}_%{buildid}_XXXXXX") \
  /* Whether to trust existing versions of extracted embedded data. */  \
  F(bool, EmbeddedDataTrustExtract,    true)                            \
  F(bool, EmitSwitch,                  true)                            \
  F(bool, LogThreadCreateBacktraces,   false)                           \
  F(bool, FailJitPrologs,              false)                           \
  F(bool, UseHHBBC,                    !getenv("HHVM_DISABLE_HHBBC"))   \
  /* Generate warning of side effect of the pseudomain is called by     \
     top-level code.*/                                                  \
  F(bool, WarnOnRealPseudomain, false)                                  \
  /* Generate warnings of side effect on top-level code that is not     \
   * called by pseudomain directly (include from other file)            \
   * 0 - Nothing                                                        \
   * 1 - Raise Warning                                                  \
   * 2 - Throw exception                                                \
   */                                                                   \
  F(int32_t, WarnOnUncalledPseudomain, 0)                               \
  /* The following option enables the runtime checks for `this` typehints.
   * There are 4 possible options:
   * 0 - No checking of `this` typehints.
   * 1 - Check `this` as hard `self` typehints.
   * 2 - Check `this` typehints as soft `this` typehints
   * 3 - Check `this` typehints as hard `this` typehints (unless explicitly
   *     soft).  This is the only option which enable optimization in HHBBC.
   */                                                                   \
  F(int32_t, ThisTypeHintLevel,        EnableHipHopSyntax ? 3 : 0)      \
  /* CheckReturnTypeHints:
     0 - No checks or enforcement for return type hints.
     1 - Raises E_WARNING if a return type hint fails.
     2 - Raises E_RECOVERABLE_ERROR if regular return type hint fails,
         raises E_WARNING if soft return type hint fails. If a regular
         return type hint fails, it's possible for execution to resume
         normally if the user error handler doesn't throw and returns
         something other than boolean false.
     3 - Same as 2, except if a regular type hint fails the runtime
         will not allow execution to resume normally; if the user
         error handler returns something other than boolean false,
         the runtime will throw a fatal error. */                       \
  F(int32_t, CheckReturnTypeHints,     2)                               \
  /*
    CheckPropTypeHints:
    0 - No checks or enforcement of property type hints.
    1 - Raises E_WARNING if a property type hint fails.
    2 - Raises E_RECOVERABLE_ERROR if regular property type hint fails, raises
        E_WARNING if soft property type hint fails. If a regular property type
        hint fails, it's possible for execution to resume normally if the user
        error handler doesn't throw and returns something other than boolean
        false.
    3 - Same as 2, except if a regular property type hint fails the runtime will
        not allow execution to resume normally; if the user error handler
        returns something other than boolean false, the runtime will throw a
        fatal error.
  */                                                                    \
  F(int32_t, CheckPropTypeHints,       0)                               \
  /* Whether or not to assume that VerifyParamType instructions must
     throw if the parameter does not match the associated type
     constraint. This changes program behavior because parameter type
     hint validation is normally a recoverable fatal. When this option
     is on, hhvm will fatal if the error handler tries to recover in
     this situation.
     1) In repo-mode, we only set this option to hphpc, and serialize
        it in Repo::GlobalData::HardTypeHints. Subsequent invocations
        to hhbbc/hhvm runtime will only load it from the GlobalData.
     2) In non-repo mode, we set this option to the runtime as usual.
     3) Both HHBBC and the runtime should query only this option
        instead of the GlobalData.
  */                                                                    \
  F(bool, HardTypeHints,               RepoAuthoritative)               \
  F(bool, PromoteEmptyObject,          !EnableHipHopSyntax)             \
  F(bool, AllowObjectDestructors,      !one_bit_refcount)               \
  F(bool, LibXMLUseSafeSubtrees,       true)                            \
  F(bool, AllDestructorsOptional,      false)                           \
  F(bool, AllowScopeBinding,           true)                            \
  F(bool, JitNoGdb,                    true)                            \
  F(bool, SpinOnCrash,                 false)                           \
  F(uint32_t, DumpRingBufferOnCrash,   0)                               \
  F(bool, PerfPidMap,                  true)                            \
  F(bool, PerfPidMapIncludeFilePath,   true)                            \
  F(bool, PerfJitDump,                 false)                           \
  F(string, PerfJitDumpDir,            "/tmp")                          \
  F(bool, PerfDataMap,                 false)                           \
  F(bool, KeepPerfPidMap,              false)                           \
  F(int32_t, PerfRelocate,             0)                               \
  F(uint32_t, ThreadTCMainBufferSize,  6 << 20)                         \
  F(uint32_t, ThreadTCColdBufferSize,  6 << 20)                         \
  F(uint32_t, ThreadTCFrozenBufferSize,4 << 20)                         \
  F(uint32_t, ThreadTCDataBufferSize,  256 << 10)                       \
  F(uint32_t, JitTargetCacheSize,      64 << 20)                        \
  F(uint32_t, HHBCArenaChunkSize,      10 << 20)                        \
  F(bool, ProfileBC,                   false)                           \
  F(bool, ProfileHeapAcrossRequests,   false)                           \
  F(bool, ProfileHWEnable,             true)                            \
  F(string, ProfileHWEvents,           std::string(""))                 \
  F(bool, ProfileHWExcludeKernel,      false)                           \
  F(bool, ProfileHWFastReads,          false)                           \
  F(bool, ProfileHWStructLog,          false)                           \
  F(int32_t, ProfileHWExportInterval,  30)                              \
  F(bool, JitAlwaysInterpOne,          false)                           \
  F(int32_t, JitNopInterval,           0)                               \
  F(uint32_t, JitMaxTranslations,      10)                              \
  F(uint32_t, JitMaxProfileTranslations, 30)                            \
  F(uint32_t, JitTraceletGuardsLimit,  5)                               \
  F(uint64_t, JitGlobalTranslationLimit, -1)                            \
  F(int64_t, JitMaxRequestTranslationTime, -1)                          \
  F(uint32_t, JitMaxRegionInstrs,      1347)                            \
  F(uint32_t, JitProfileInterpRequests, kDefaultProfileInterpRequests)  \
  F(uint32_t, JitMaxAwaitAllUnroll,    8)                               \
  F(bool, JitProfileWarmupRequests,    false)                           \
  F(uint32_t, NumSingleJitRequests,    nsjrDefault())                   \
  F(uint32_t, JitProfileRequests,      profileRequestsDefault())        \
  F(uint32_t, JitProfileBCSize,        profileBCSizeDefault())          \
  F(uint32_t, JitResetProfCountersRequest, resetProfCountersDefault())  \
  F(uint32_t, JitRetranslateAllRequest, retranslateAllRequestDefault()) \
  F(uint32_t, JitRetranslateAllSeconds, retranslateAllSecondsDefault()) \
  F(bool,     JitLayoutSplitHotCold,   layoutSplitHotColdDefault())     \
  F(double,   JitLayoutHotThreshold,   0.05)                            \
  F(int32_t,  JitLayoutMainFactor,     1000)                            \
  F(int32_t,  JitLayoutColdFactor,     5)                               \
  F(bool,     JitAHotSizeRoundUp,      true)                            \
  F(bool, JitProfileRecord,            false)                           \
  F(uint32_t, GdbSyncChunks,           128)                             \
  F(bool, JitKeepDbgFiles,             false)                           \
  /* despite the unfortunate name, this enables function renaming and
   * interception in the interpreter as well as the jit, and also
   * implies all functions may be used with fb_intercept */             \
  F(bool, JitEnableRenameFunction,     EvalJitEnableRenameFunction)     \
  F(bool, JitUseVtuneAPI,              false)                           \
                                                                        \
  F(bool, JitDisabledByHphpd,          false)                           \
  F(bool, JitPseudomain,               true)                            \
  F(uint32_t, JitWarmupStatusBytes,    ((25 << 10) + 1))                \
  F(uint32_t, JitWarmupMaxCodeGenRate, 20000)                           \
  F(uint32_t, JitWarmupRateSeconds,    64)                              \
  F(uint32_t, JitWriteLeaseExpiration, 1500) /* in microseconds */      \
  F(int, JitRetargetJumps,             1)                               \
  F(bool, HHIRLICM,                    false)                           \
  F(bool, HHIRSimplification,          true)                            \
  F(bool, HHIRGenOpts,                 true)                            \
  F(bool, HHIRRefcountOpts,            true)                            \
  F(bool, HHIREnableGenTimeInlining,   true)                            \
  F(uint32_t, HHIRInliningVasmCostLimit, 175)                           \
  F(uint32_t, HHIRInliningMinVasmCostLimit, 100)                        \
  F(uint32_t, HHIRInliningMaxVasmCostLimit, 400)                        \
  F(double,   HHIRInliningVasmCallerExp, .5)                            \
  F(double,   HHIRInliningVasmCalleeExp, .5)                            \
  F(uint32_t, HHIRInliningMaxReturnDecRefs, 12)                         \
  F(uint32_t, HHIRInliningMaxReturnLocals, 20)                          \
  F(bool,     HHIRInliningIgnoreHints, !debug)                          \
  F(bool, HHIRInlineFrameOpts,         true)                            \
  F(bool, HHIRPartialInlineFrameOpts,  true)                            \
  F(bool, HHIRInlineSingletons,        true)                            \
  F(std::string, InlineRegionMode,     "both")                          \
  F(bool, HHIRGenerateAsserts,         false)                           \
  F(bool, HHIRDeadCodeElim,            true)                            \
  F(bool, HHIRGlobalValueNumbering,    true)                            \
  F(bool, HHIRPredictionOpts,          true)                            \
  F(bool, HHIRMemoryOpts,              true)                            \
  F(bool, AssemblerFoldDefaultValues,  true)                            \
  F(uint32_t, HHIRLoadElimMaxIters,    10)                              \
  F(bool, HHIRStorePRE,                true)                            \
  F(bool, HHIROutlineGenericIncDecRef, true)                            \
  F(double, HHIRMixedArrayProfileThreshold, 0.8554)                     \
  /* Register allocation flags */                                       \
  F(bool, HHIREnablePreColoring,       true)                            \
  F(bool, HHIREnableCoalescing,        true)                            \
  F(bool, HHIRAllocSIMDRegs,           true)                            \
  F(bool, HHIRStressSpill,             false)                           \
  /* Region compiler flags */                                           \
  F(string,   JitRegionSelector,       regionSelectorDefault())         \
  F(bool,     JitPGO,                  pgoDefault())                    \
  F(string,   JitPGORegionSelector,    "hotcfg")                        \
  F(uint64_t, JitPGOThreshold,         pgoThresholdDefault())           \
  F(bool,     JitPGOOnly,              false)                           \
  F(bool,     JitPGOHotOnly,           false)                           \
  F(bool,     JitPGOUsePostConditions, true)                            \
  F(uint32_t, JitPGOUnlikelyIncRefCountedPercent, 2)                    \
  F(uint32_t, JitPGOUnlikelyIncRefIncrementPercent, 5)                  \
  F(uint32_t, JitPGOUnlikelyDecRefReleasePercent, 5)                    \
  F(uint32_t, JitPGOUnlikelyDecRefCountedPercent, 2)                    \
  F(uint32_t, JitPGOUnlikelyDecRefPersistPercent, 5)                    \
  F(uint32_t, JitPGOUnlikelyDecRefSurvivePercent, 5)                    \
  F(uint32_t, JitPGOUnlikelyDecRefDecrementPercent, 5)                  \
  F(uint32_t, JitPGOReleaseVVMinPercent, 8)                             \
  F(bool,     JitPGOArrayGetStress,    false)                           \
  F(uint32_t, JitPGOMinBlockCountPercent, 0)                            \
  F(double,   JitPGOMinArcProbability, 0.0)                             \
  F(uint32_t, JitPGOMaxFuncSizeDupBody, 80)                             \
  F(uint32_t, JitPGORelaxPercent,      100)                             \
  F(uint32_t, JitPGORelaxUncountedToGenPercent, 20)                     \
  F(uint32_t, JitPGORelaxCountedToGenPercent, 75)                       \
  F(uint32_t, JitPGOBindCallThreshold, 50)                              \
  F(double,   JitPGOPushedFuncThreshold, 99.9)                          \
  F(bool,     JitPGODumpCallGraph,     false)                           \
  F(bool,     JitPGORacyProfiling,     false)                           \
  F(uint64_t, FuncCountHint,           10000)                           \
  F(uint64_t, PGOFuncCountHint,        1000)                            \
  F(uint32_t, HotFuncCount,            4100)                            \
  F(bool, RegionRelaxGuards,           true)                            \
  /* DumpBytecode =1 dumps user php, =2 dumps systemlib & user php */   \
  F(int32_t, DumpBytecode,             0)                               \
  /* DumpHhas =1 dumps user php, =2 dumps systemlib & user php */       \
  F(int32_t, DumpHhas,                 0)                               \
  F(string, DumpHhasToFile,            "")                              \
  F(bool, DisableHphpcOpts,            false)                           \
  F(bool, DisableErrorHandler,         false)                           \
  F(bool, DumpTC,                      false)                           \
  F(string, DumpTCPath,                "/tmp")                          \
  F(bool, DumpTCAnchors,               false)                           \
  F(uint32_t, DumpIR,                  0)                               \
  F(bool, DumpTCAnnotationsForAllTrans,debug)                           \
  F(bool, DumpInlRefuse,               false)                           \
  F(uint32_t, DumpRegion,              0)                               \
  F(bool, DumpAst,                     false)                           \
  F(bool, DumpTargetProfiles,          false)                           \
  F(bool, MapTgtCacheHuge,             false)                           \
  F(uint32_t, MaxHotTextHugePages,     hotTextHugePagesDefault())       \
  F(int32_t, MaxLowMemHugePages,       hugePagesSoundNice() ? 8 : 0)    \
  F(uint32_t, Num1GPagesForSlabs,      0)                               \
  F(uint32_t, Num2MPagesForSlabs,      0)                               \
  F(bool, LowStaticArrays,             true)                            \
  F(int64_t, HeapPurgeWindowSize,      5 * 1000000)                     \
  F(uint64_t, HeapPurgeThreshold,      128 * 1024 * 1024)               \
  /* GC Options: See heap-collect.cpp for more details */               \
  F(bool, EagerGC,                     eagerGcDefault())                \
  F(bool, FilterGCPoints,              true)                            \
  F(bool, Quarantine,                  eagerGcDefault())                \
  F(uint32_t, GCSampleRate,            0)                               \
  F(int64_t, GCMinTrigger,             64L<<20)                         \
  F(double, GCTriggerPct,              0.5)                             \
  F(bool, GCForAPC,                    false)                           \
  F(int64_t, GCForAPCTrigger,          1024*1024*1024)                  \
  F(bool, TwoPhaseGC,                  false)                           \
  F(bool, EnableGC,                    enableGcDefault())               \
  /* End of GC Options */                                               \
  F(bool, RaiseMissingThis,            !EnableHipHopSyntax)             \
  F(bool, QuoteEmptyShellArg,          !EnableHipHopSyntax)             \
  F(bool, Verify,                      (getenv("HHVM_VERIFY") ||        \
    !EvalHackCompilerCommand.empty()))                                  \
  F(bool, VerifyOnly,                  false)                           \
  F(bool, FatalOnVerifyError,          !RepoAuthoritative)              \
  F(bool, AbortBuildOnVerifyError,     true)                            \
  F(uint32_t, StaticContentsLogRate,   100)                             \
  F(uint32_t, LogUnitLoadRate,         0)                               \
  F(uint32_t, MaxDeferredErrors,       50)                              \
  F(bool, JitAlignMacroFusionPairs, alignMacroFusionPairs())            \
  F(bool, JitAlignUniqueStubs,         true)                            \
  F(uint32_t, SerDesSampleRate,            0)                           \
  F(int, SimpleJsonMaxLength,        2 << 20)                           \
  F(uint32_t, JitSampleRate,               0)                           \
  /* Log the sizes and metadata for all translations in the TC broken
   * down by function and inclusive/exclusive size for inlined regions.
   * When set to "" TC size data will be sampled on a per function basis
   * as determined by JitSampleRate. When set to a non-empty string all
   * translations will be logged, and run_key column will be logged with
   * the value of this option. */                                       \
  F(string,   JitLogAllInlineRegions,  "")                              \
  F(bool, JitProfileGuardTypes,        false)                           \
  F(uint32_t, JitFilterLease,          1)                               \
  F(bool, DisableSomeRepoAuthNotices,  true)                            \
  F(uint32_t, PCRETableSize, kPCREInitialTableSize)                     \
  F(uint64_t, PCREExpireInterval, 2 * 60 * 60)                          \
  F(string, PCRECacheType, std::string("static"))                       \
  F(bool, EnableCompactBacktrace, true)                                 \
  F(bool, EnableNuma, ServerExecutionMode())                            \
  F(bool, EnableNumaLocal, ServerExecutionMode())                       \
  /* Use 1G pages for jemalloc metadata. */                             \
  F(bool, EnableArenaMetadata1GPage, false)                             \
  /* Use 1G pages for jemalloc metadata (NUMA arenas if applicable). */ \
  F(bool, EnableNumaArenaMetadata1GPage, false)                         \
  /* Reserved space on 1G pages for jemalloc metadata (arena0). */      \
  F(uint64_t, ArenaMetadataReservedSize, 216 << 20)                     \
  F(bool, EnableCallBuiltin, true)                                      \
  F(bool, EnableReusableTC,   reuseTCDefault())                         \
  F(bool, LogServerRestartStats, false)                                 \
  F(uint32_t, ReusableTCPadding, 128)                                   \
  F(int64_t,  StressUnitCacheFreq, 0)                                   \
  F(int64_t, PerfWarningSampleRate, 1)                                  \
  F(int64_t, FunctionCallSampleRate, 0)                                 \
  F(double, InitialLoadFactor, 1.0)                                     \
  /* Raise notices on various array operations which may present        \
   * compatibility issues with Hack arrays.                             \
   *                                                                    \
   * The various *Notices options independently control separate        \
   * subsets of notices.  The Check* options are subordinate to the     \
   * HackArrCompatNotices option, and control whether various runtime
   * checks are made; they do not affect any optimizations. */          \
  F(bool, HackArrCompatNotices, false)                                  \
  F(bool, HackArrCompatCheckIntishCast, false)                          \
  F(bool, HackArrCompatCheckRefBind, false)                             \
  F(bool, HackArrCompatCheckFalseyPromote, false)                       \
  F(bool, HackArrCompatCheckCompare, false)                             \
  F(bool, HackArrCompatCheckMisc, false)                                \
  /* Raise notices when is_array is called with any hack array */       \
  F(bool, HackArrCompatIsArrayNotices, false)                           \
  /* Raise notices when is_vec or is_dict  is called with a v/darray */ \
  F(bool, HackArrCompatIsVecDictNotices, false)                         \
  F(bool, HackArrCompatPromoteNotices, false)                           \
  F(bool, HackArrCompatTypeHintNotices, false)                          \
  F(bool, HackArrCompatDVCmpNotices, false)                             \
  F(bool, HackArrCompatSerializeNotices, false)                         \
  F(bool, HackArrDVArrs, false)                                         \
  /* Warn if is expression are used with type aliases that cannot be    |
   * resolved */                                                        \
  F(bool, IsExprEnableUnresolvedWarning, false)                         \
  /* Switches on miscellaneous junk. */                                 \
  F(bool, NoticeOnCreateDynamicProp, false)                             \
  F(bool, NoticeOnReadDynamicProp, false)                               \
  F(bool, CreateInOutWrapperFunctions, true)                            \
  F(bool, ReffinessInvariance, false)                                   \
  F(bool, NoticeOnBuiltinDynamicCalls, false)                           \
  F(bool, RxPretendIsEnabled, false)                                    \
  /* Raise warning when function pointers are used as strings. */       \
  F(bool, RaiseFuncConversionWarning, false)                            \
  /* Raise warning when class pointers are used as strings. */          \
  F(bool, RaiseClassConversionWarning, false)                           \
  /*                                                                    \
   * Control dynamic calls to functions which haven't opted into being called \
   * that way.                                                          \
   *                                                                    \
   * 0 - Nothing                                                        \
   * 1 - Warn                                                           \
   * 2 - Throw exception                                                \
   *                                                                    \
   */                                                                   \
  F(int32_t, ForbidDynamicCalls, 0)                                     \
  F(int32_t, ServerOOMAdj, 0)                                           \
  F(std::string, PreludePath, "")                                       \
  F(uint32_t, NonSharedInstanceMemoCaches, 10)                          \
  F(std::vector<std::string>, IniGetHide, std::vector<std::string>())   \
  F(std::string, UseRemoteUnixServer, "no")                             \
  F(std::string, UnixServerPath, "")                                    \
  F(uint32_t, UnixServerWorkers, Process::GetCPUCount())                \
  F(bool, UnixServerQuarantineApc, false)                               \
  F(bool, UnixServerQuarantineUnits, false)                             \
  F(bool, UnixServerVerifyExeAccess, false)                             \
  F(bool, UnixServerFailWhenBusy, false)                                \
  F(std::vector<std::string>, UnixServerAllowedUsers,                   \
                                            std::vector<std::string>()) \
  F(std::vector<std::string>, UnixServerAllowedGroups,                  \
                                            std::vector<std::string>()) \
  /* Options for testing */                                             \
  F(bool, TrashFillOnRequestExit, false)                                \
  /******************                                                   \
   | ARM   Options. |                                                   \
   *****************/                                                   \
  F(bool, JitArmLse, armLseDefault())                                   \
  /******************                                                   \
   | PPC64 Options. |                                                   \
   *****************/                                                   \
  /* Minimum immediate size to use TOC */                               \
  F(uint16_t, PPC64MinTOCImmSize, 64)                                   \
  /* Relocation features. Use with care on production */                \
  /*  Allow a Far branch be converted to a Near branch. */              \
  F(bool, PPC64RelocationShrinkFarBranches, false)                      \
  /*  Remove nops from a Far branch. */                                 \
  F(bool, PPC64RelocationRemoveFarBranchesNops, true)                   \
  /********************                                                 \
   | Profiling flags. |                                                 \
   ********************/                                                \
  /* Whether to maintain the address-to-VM-object mapping. */           \
  F(bool, EnableReverseDataMap, true)                                   \
  /* Turn on perf-mem-event sampling roughly every this many requests.  \
   * To maintain the same overall sampling rate, the ratio between the  \
   * request and sample frequencies should be kept constant. */         \
  F(uint32_t, PerfMemEventRequestFreq, 0)                               \
  /* Sample this many memory instructions per second.  This should be   \
   * kept low to avoid the risk of collecting a sample while we're      \
   * processing a previous sample. */                                   \
  F(uint32_t, PerfMemEventSampleFreq, 80)                               \
  /* Sampling frequency for TC branch profiling. */                     \
  F(uint32_t, ProfBranchSampleFreq, 0)                                  \
  /* Sampling frequency for profiling packed array accesses. */         \
  F(uint32_t, ProfPackedArraySampleFreq, 0)                             \
  /* */

private:
  using string = std::string;

  // Custom settings. This should be accessed via the GetServerCustomSetting
  // APIs.
  static std::map<std::string, std::string> CustomSettings;

public:
#define F(type, name, unused) \
  static type Eval ## name;
  EVALFLAGS()
#undef F

  static bool RecordCodeCoverage;
  static std::string CodeCoverageOutputFile;

  // Repo (hhvm bytecode repository) options
  static std::string RepoLocalMode;
  static std::string RepoLocalPath;
  static std::string RepoCentralPath;
  static int32_t RepoCentralFileMode;
  static std::string RepoCentralFileUser;
  static std::string RepoCentralFileGroup;
  static bool RepoAllowFallbackPath;
  static std::string RepoEvalMode;
  static std::string RepoJournal;
  static bool RepoCommit;
  static bool RepoDebugInfo;
  static bool RepoAuthoritative;
  static bool RepoPreload;
  static int64_t RepoLocalReadaheadRate;
  static bool RepoLocalReadaheadConcurrent;

  // pprof/hhprof options
  static bool HHProfEnabled;
  static bool HHProfActive;
  static bool HHProfAccum;
  static bool HHProfRequest;
  static bool TrackPerUnitMemory;

  // Sandbox options
  static bool SandboxMode;
  static std::string SandboxPattern;
  static std::string SandboxHome;
  static std::string SandboxFallback;
  static std::string SandboxConfFile;
  static std::map<std::string, std::string> SandboxServerVariables;
  static bool SandboxFromCommonRoot;
  static std::string SandboxDirectoriesRoot;
  static std::string SandboxLogsRoot;

  // Debugger options
  static bool EnableHphpdDebugger;
  static bool EnableVSDebugger;
  static int VSDebuggerListenPort;
  static bool VSDebuggerNoWait;
  static bool EnableDebuggerColor;
  static bool EnableDebuggerPrompt;
  static bool EnableDebuggerServer;
  static bool EnableDebuggerUsageLog;
  static bool DebuggerDisableIPv6;
  static std::string DebuggerServerIP;
  static int DebuggerServerPort;
  static int DebuggerDefaultRpcPort;
  static std::string DebuggerDefaultRpcAuth;
  static std::string DebuggerRpcHostDomain;
  static int DebuggerDefaultRpcTimeout;
  static std::string DebuggerDefaultSandboxPath;
  static std::string DebuggerStartupDocument;
  static int DebuggerSignalTimeout;
  static std::string DebuggerAuthTokenScriptBin;

  // Mail options
  static std::string SendmailPath;
  static std::string MailForceExtraParameters;

  // preg stack depth and debug support options
  static int64_t PregBacktraceLimit;
  static int64_t PregRecursionLimit;
  static bool EnablePregErrorLog;

  // SimpleXML options
  static bool SimpleXMLEmptyNamespaceMatchesAll;

  // Cookie options
  static bool AllowDuplicateCookies;

#ifdef FACEBOOK
  // fb303 server
  static bool EnableFb303Server;
  static int Fb303ServerPort;
  static int Fb303ServerThreadStackSizeMb;
  static int Fb303ServerWorkerThreads;
  static int Fb303ServerPoolThreads;
#endif

  // Xenon options
  static double XenonPeriodSeconds;
  static bool XenonForceAlwaysOn;
  static bool XenonTraceUnitLoad;
  static std::string XenonStructLogDest;
};
static_assert(sizeof(RuntimeOption) == 1, "no instance variables");

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_RUNTIME_OPTION_H_
