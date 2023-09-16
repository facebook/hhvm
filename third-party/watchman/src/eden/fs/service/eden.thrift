/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This software may be used and distributed according to the terms of the
 * GNU General Public License version 2.
 */

include "eden/fs/config/eden_config.thrift"
include "fb303/thrift/fb303_core.thrift"

namespace cpp2 facebook.eden
namespace java com.facebook.eden.thrift
namespace py facebook.eden
namespace py3 eden.fs.service
namespace hack edenfs.service

/**
 * API style guide.
 * ----------------
 *
 * These guides are to ensure we use consitent pratices to make
 * our interface easy to use.
 * 1. Wrap the endpoint arguments in a struct. The name of this argument
 * struct should be the endpointname + "Request". This is Thrift's recommended
 * practice for arguments and ensures that we can safely evolve the arguments
 * of a method.
 * 2. Wrap the return value from the endpoint in a struct, even if it is just
 * a single value. This allows evolving return types without having to create a
 * whole new endpoint. The name of this return value struct should be
 * endpointname + "Response".
 * 3. If your endpoint operates on a mount(s), make the mount identifier
 * the first value in your arguments struct. Use the MountId struct as the type.
 * This allows us to evolve the identifiers we use for mountpoints in the future
 * without rewriting your new endpoint.
 */

/** Thrift doesn't really do unsigned numbers, but we can sort of fake it.
 * This type is serialized as an integer value that is 64-bits wide and
 * should round-trip with full fidelity for C++ client/server, but for
 * other runtimes will have crazy results if the sign bit is ever set.
 * In practice it is impossible for us to have files that large in eden,
 * and sequence numbers will take an incredibly long time to ever roll
 * over and cause problems.
 * Once t13345978 is done, we can uncomment the cpp.type below.
 */
typedef i64 /* (cpp.type = "std::uint64_t") */
unsigned64

typedef i32 pid_t

/**
 * A backing-store-specific identifier for the root tree. For Mercurial or
 * Git, this is a 20-byte binary hash or a 40-byte hexadecimal hash.
 *
 * For other backing stores, this string may have variable length and its
 * meaning is defined by the backing-store. If possible, prefer human-readable
 * strings so they can be read in log files and error messages.
 *
 * This is named ThriftRootId to not conflict with the RootId class, but
 * perhaps this Thrift module should be placed in its own namespace.
 */
typedef binary ThriftRootId

typedef binary ThriftObjectId

/**
 * A source control hash.
 *
 * This should normally be a 20-byte binary value, however the edenfs server
 * will accept BinaryHash arguments as 40-byte hexadecimal strings as well.
 * Data returned by the edenfs server in a BinaryHash field will always be a
 * 20-byte binary value.
 */
typedef binary BinaryHash

/**
 * So, you thought that a path was a string?
 * Paths in posix are arbitrary byte strings with some pre-defined special
 * characters.  On modern systems they tend to be represented as UTF-8 but
 * there is no guarantee.  We use the `PathString` type as symbolic way
 * to indicate that you may need to perform special processing to safely
 * interpret the path data on your system.
 */
typedef binary PathString

/**
 * Bit set indicating where data should be fetched from in our debugging
 * commands.
 * Bits are defined by DataFetchOriginSet.
 */
typedef unsigned64 DataFetchOriginSet

/**
 * A customizable type to be returned with an EdenError, helpful for catching
 * and having custom client logic to handle specfic error cases
 */
enum EdenErrorType {
  /** The errorCode property is a posix errno value */
  POSIX_ERROR = 0,
  /** The errorCode property is a win32 error value */
  WIN32_ERROR = 1,
  /** The errorCode property is a windows NT HResult error value */
  HRESULT_ERROR = 2,
  /**
   * An argument passed to thrift was invalid. errorCode will be set to EINVAL
   */
  ARGUMENT_ERROR = 3,
  /** An error occurred. errorCode will be not set */
  GENERIC_ERROR = 4,
  /** The mount generation changed. errorCode will be set to ERANGE */
  MOUNT_GENERATION_CHANGED = 5,
  /** The journal has been truncated. errorCode will be set to EDOM */
  JOURNAL_TRUNCATED = 6,
  /**
   * The thrift funtion that receives this in an error is being called while
   * a checkout is in progress. errorCode will not be set.
   */
  CHECKOUT_IN_PROGRESS = 7,
  /**
  * The thrift function that receives this is an error is being called with a
  * parent that is not the current parent. errorCode will not be set.
  */
  OUT_OF_DATE_PARENT = 8,
}

exception EdenError {
  1: string message;
  2: optional i32 errorCode;
  3: EdenErrorType errorType;
} (message = 'message')

exception NoValueForKeyError {
  1: string key;
}

struct MountId {
  1: PathString mountPoint;
}

/**
 * Information about the running edenfs daemon.
 */
struct DaemonInfo {
  1: i32 pid;
  /**
   * List of command line arguments, including the executable name,
   * given to the edenfs process.
   */
  2: list<string> commandLine;
  /**
   * The service status.
   *
   * This is almost the same value reported by
   * fb303_core.BaseService.getStatus(). fb303_core.BaseService.getStatus()
   * only returns the Thrift server status and does not understand mounts or
   * graceful restarts.
   */
  3: optional fb303_core.fb303_status status;
  /**
   * The uptime of the edenfs daemon
   * Same data from /proc/pid/stat
   */
  4: optional float uptime;
}

/**
* Information about the privhelper process
*/
struct PrivHelperInfo {
  1: bool connected;
}

/**
 * The current running state of an EdenMount.
 */
enum MountState {
  /**
   * The EdenMount object has been constructed but has not started
   * initializing.
   */
  UNINITIALIZED = 0,
  /**
   * The mount point is currently initializing and loading necessary state
   * (such as the root directory contents) before it can ask the kernel to
   * mount it.
   */
  INITIALIZING = 1,
  /**
   * The mount point has loaded its local state needed to start mounting
   * but has not actually started mounting yet.
   */
  INITIALIZED = 2,
  /**
   * Starting to mount the filesystem.
   */
  STARTING = 3,
  /**
   * The EdenMount is running normally.
   */
  RUNNING = 4,
  /**
   * Encountered an error while starting the user-space filesystem mount.
   * FUSE is a misnomer: this error state also applies to NFS and Windows'
   * Projected File System.
   * TODO: rename to INITIALIZATION_ERROR.
   */
  FUSE_ERROR = 5,
  /**
   * EdenMount::shutdown() has been called, but it is not complete yet.
   */
  SHUTTING_DOWN = 6,
  /**
   * EdenMount::shutdown() has completed, but there are still outstanding
   * references so EdenMount::destroy() has not been called yet.
   *
   * When EdenMount::destroy() is called the object can be destroyed
   * immediately.
   */
  SHUT_DOWN = 7,
  /**
   * EdenMount::destroy() has been called, but the shutdown is not complete
   * yet.  There are no remaining references to the EdenMount at this point,
   * so when the shutdown completes it will be automatically destroyed.
   */
  DESTROYING = 8,
  /**
   * An error occurred during mount initialization.
   *
   * This state is used for errors that occur during the INITIALIZING phase,
   * before we have attempted to start the user-space filesystem mount.
   */
  INIT_ERROR = 9,
} (cpp2.enum_type = 'uint32_t')

struct MountInfo {
  1: PathString mountPoint;
  2: PathString edenClientPath;
  3: MountState state;
  4: optional PathString backingRepoPath;
}

struct MountArgument {
  1: PathString mountPoint;
  2: PathString edenClientPath;
  3: bool readOnly;
}

union SHA1Result {
  1: BinaryHash sha1;
  2: EdenError error;
}

union Blake3Result {
  1: BinaryHash blake3;
  2: EdenError error;
}

/**
 * Effectively a `struct timespec`
 */
struct TimeSpec {
  1: i64 seconds;
  2: i64 nanoSeconds;
}

/**
 * Information about filesystem entries that can be retrieved solely
 * from the tree structure, without having to fetch the actual child
 * objects from source control.
 */
struct EntryInformation {
  1: Dtype dtype;
}

union EntryInformationOrError {
  1: EntryInformation info;
  2: EdenError error;
}

/**
 * Subset of stat() data returned from getFileInformation())
 */
struct FileInformation {
  1: unsigned64 size; // wish thrift had unsigned numbers
  2: TimeSpec mtime;
  3: i32 mode; // mode_t
}

/** Holds information about a file, or an error in retrieving that info.
 * The most likely error will be ENOENT, implying that the file doesn't exist.
 */
union FileInformationOrError {
  1: FileInformation info;
  2: EdenError error;
}

/**
 * File Attributes that can be requested with getAttributesFromFiles(). All attributes
 * should be a power of 2. OR the requested attributes together to get a bitmask.
 */
enum FileAttributes {
  NONE = 0,
  /**
   * Returns the SHA-1 hash of a file. Returns an error for symlinks and directories,
   * and non-regular files.
   */
  SHA1_HASH = 1,
  /**
   * Returns the size of a file. Returns an error for symlinks and directories.
   */
  FILE_SIZE = 2,
  /**
   * Returns the type of a file or directory if it has a corresponding "source
   * control type" that can be represented in a source control type.
   */
  SOURCE_CONTROL_TYPE = 4,
  /**
   * Returns an opaque identifier that can be used to know when a file or directory
   * (recursively) has changed.
   *
   * If the file or directory (recursively) has been locally written to, no object ID
   * will be returned. In that case, the caller should physically compare or search
   * the directory structure.
   *
   * Do not attempt to parse this identifier. It is subject to change at any moment.
   */
  OBJECT_ID = 8,

  /**
   * Returns the BLAKE3 hash of a file. Returns an error for symlinks and directories,
   * and non-regular files.
   */
  BLAKE3_HASH = 16,
/* NEXT_ATTR = 2^x */
} (cpp2.enum_type = 'uint64_t')

typedef unsigned64 RequestedAttributes

/**
 * Subset of attributes for a single file returned by getAttributesFromFiles()
 */
struct FileAttributeData {
  1: optional BinaryHash sha1;
  2: optional i64 fileSize;
  3: optional SourceControlType type;
}

/**
 * Attributes for a file or information about error encountered when accessing file attributes.
 * The most likely error will be ENOENT, implying that the file doesn't exist.
 */
union FileAttributeDataOrError {
  1: FileAttributeData data;
  2: EdenError error;
}

/**
 * Some attributes are not available for certain types of files. For example,
 * sha1 and size are not available for directories or symlinks. We use a value
 * or error type for each of those attributes, so that when we can not provide
 * an attribute we can give an explanation for why not.
 */
union Sha1OrError {
  1: BinaryHash sha1;
  2: EdenError error;
}

union Blake3OrError {
  1: BinaryHash blake3;
  2: EdenError error;
}

union SizeOrError {
  1: i64 size;
  2: EdenError error;
}

union SourceControlTypeOrError {
  1: SourceControlType sourceControlType;
  2: EdenError error;
}

union ObjectIdOrError {
  // If the path has been locally written to, it will have no object ID. Therefore,
  // it's possible for `objectId` to be unset even if there is no error.
  //
  // Notably, if path refers to a directory, no object ID will be returned if any
  // child file or directory has been written to.
  1: ThriftObjectId objectId;
  2: EdenError error;
}

/**
 * Subset of attributes for a single file returned by getAttributesFromFiles()
 *
 * When an attribute was not requested the field will be a null optional value.
 * If the attribute was requested, but there was an error computing that
 * specific attribute we will return an Error type for that attribute.
 */
struct FileAttributeDataV2 {
  1: optional Sha1OrError sha1;
  2: optional SizeOrError size;
  3: optional SourceControlTypeOrError sourceControlType;
  4: optional ObjectIdOrError objectId;
  5: optional Blake3OrError blake3;
}

/**
 * Attributes for a file or information about error encountered when accessing
 * file attributes.
 * If there were errors fetching particular attributes those will be encapsulated
 * in the AttributeOrError type. If there was a general error accessing a file
 * there will be an error here.
 */
union FileAttributeDataOrErrorV2 {
  1: FileAttributeDataV2 fileAttributeData;
  2: EdenError error;
}

/**
 * Mapping from entry name to requested attributes for each of the entries
 * in a certain directory.
 */
union DirListAttributeDataOrError {
  1: map<PathString, FileAttributeDataOrErrorV2> (
    rust.type = "sorted_vector_map::SortedVectorMap",
  ) dirListAttributeData;
  2: EdenError error;
}

/**
 *
 * Ensure that all inflight working copy modification have completed.
 *
 * On some platforms, EdenFS is processing working copy modifications
 * callbacks from the platform in an asynchronous manner, which means that by
 * the time a write/creat/mkdir/unlink/etc syscall returns from the kernel,
 * EdenFS may not have updated its internal state.
 *
 * Thus, an application making changes to the working copy and quickly
 * requesting EdenFS to perform an operation on it will race with EdenFS
 * updating its internal state and may thus get stale data.
 *
 * To avoid this, EdenFS queries need to internally synchronize the working
 * copy before performing the query itself. This structure defines how EdenFS
 * will do this.
 *
 * Applications that care about synchronizing EdenFS up to a certain point in
 * time are expected to set a non-zero syncTimeout once to synchronize EdenFS
 * and then issue all their thrift requests with a syncTimeout of 0.
 */
struct SyncBehavior {
  // How many seconds to wait for the working copy to be synchronized. A value
  // of 0 will not perform any working copy synchronization.
  // If left unset, EdenFS will use a default synchronization timeout.
  1: optional i64 syncTimeoutSeconds;
}

/**
 * Parameters for the getAttributesFromFiles() function
 */
struct GetAttributesFromFilesParams {
  1: PathString mountPoint;
  2: list<PathString> paths;
  3: RequestedAttributes requestedAttributes;
  4: SyncBehavior sync;
}

/**
 * Return value for the getAttributesFromFiles() function.
 * The returned list of attributes corresponds to the input list of
 * paths; eg; res[0] holds the information for paths[0].
 */
struct GetAttributesFromFilesResult {
  1: list<FileAttributeDataOrError> res;
}
struct GetAttributesFromFilesResultV2 {
  1: list<FileAttributeDataOrErrorV2> res;
}

struct ReaddirParams {
  1: PathString mountPoint;
  2: list<PathString> directoryPaths;
  3: RequestedAttributes requestedAttributes;
  4: SyncBehavior sync;
}

/**
 * List of attributes for the entries in the directories specified in
 * directoryPaths. The ordering of the responses corresponds to the ordering
 * the directoryPaths.
 */
struct ReaddirResult {
  1: list<DirListAttributeDataOrError> dirLists;
}

/** reference a point in time in the journal.
 * This can be used to reason about a point in time in a given mount point.
 * The mountGeneration value is opaque to the client.
 */
struct JournalPosition {
  /** An opaque but unique number within the scope of a given mount point.
   * This is used to determine when sequenceNumber has been invalidated. */
  1: i64 mountGeneration;

  /** Monotonically incrementing number
   * Each journalled change causes this number to increment. */
  2: unsigned64 sequenceNumber;

  /** Records the snapshot hash at the appropriate point in the journal */
  3: ThriftRootId snapshotHash;
}

/**
 * Holds information about a set of paths that changed between two points.
 * fromPosition, toPosition define the time window.
 * paths holds the list of paths that changed in that window.
 *
 * This type is quasi-deprecated. It has multiple API problems and should be
 * rethought when we have a chance to make a breaking change.
 */
struct FileDelta {
  /**
   * The fromPosition passed to getFilesChangedSince
   */
  1: JournalPosition fromPosition;
  /**
   * The current position at the time that getFilesChangedSince was called
  */
  2: JournalPosition toPosition;
  /**
   * The union of changedPaths and createdPaths contains the total set of paths
   * changed in the overlay between fromPosition and toPosition.
   * Disjoint with createdPaths.
   */
  3: list<PathString> changedPaths;
  /**
   * The set of paths created between fromPosition and toPosition.
   * Used by Watchman to search for cookies and to populate its 'new' field.
   * Disjoint with changedPaths.
   */
  4: list<PathString> createdPaths;
  /**
   * Deprecated - always empty.
   */
  5: list<PathString> removedPaths;
  /**
   * When fromPosition.snapshotHash != toPosition.snapshotHash this holds
   * the union of the set of files whose ScmFileStatus differed from the
   * committed fromPosition hash before the hash changed, and the set of
   * files whose ScmFileStatus differed from the committed toPosition hash
   * after the hash was changed.  This list of files represents files
   * whose state may have changed as part of an update operation, but
   * in ways that may not be able to be extracted solely by performing
   * source control diff operations on the from/to hashes.
   */
  6: list<PathString> uncleanPaths;
  /**
   * Contains the list of commit transitions in this range. If only files
   * have been changed, the list has one entry. Otherwise, it has size N + 1,
   * where N is the number of checkout operations.
   *
   * This list's items may not be unique: [A, B, A] is a common sequence,
   * and [A, B, C] has a different meaning than [A, C, B].
   *
   * Subsumes fromPosition.snapshotHash and toPosition.snapshotHash.
   */
  7: list<ThriftRootId> snapshotTransitions;
}

struct DebugGetRawJournalParams {
  1: PathString mountPoint;
  2: optional i32 limit;
  3: i32 fromSequenceNumber;
}

struct DebugPathChangeInfo {
  1: bool existedBefore;
  2: bool existedAfter;
}

/**
 * A fairly direct modeling of the underlying JournalDelta data structure.
 */
struct DebugJournalDelta {
  1: JournalPosition fromPosition;
  2: JournalPosition toPosition;
  3: map<PathString, DebugPathChangeInfo> changedPaths;
  4: set<PathString> uncleanPaths;
}

struct DebugGetRawJournalResponse {
  2: list<DebugJournalDelta> allDeltas;
}

/**
 * Classifies the change of the state of a file between and old and new state
 * of the repository. Most commonly, the "old state" is the parent commit while
 * the "new state" is the working copy.
 */
enum ScmFileStatus {
  /**
   * File is present in the new state, but was not present in old state.
   */
  ADDED = 0x0,

  /**
   * File is present in both the new and old states, but its contents or
   * file permissions have changed.
   */
  MODIFIED = 0x1,

  /**
   * File was present in the old state, but is not present in the new state.
   */
  REMOVED = 0x2,

  /**
   * File is present in the new state, but it is ignored according to the rules
   * of the new state.
   */
  IGNORED = 0x3,
}

struct ScmStatus {
  1: map<PathString, ScmFileStatus> entries;

  /**
   * A map of { path -> error message }
   *
   * If any errors occured while computing the diff they will be reported here.
   * The results listed in the entries field may not be accurate for any paths
   * listed in this error field.
   *
   * This map will be empty if no errors occurred.
   */
  2: map<PathString, string> errors;
}

/** Option for use with checkOutRevision(). */
enum CheckoutMode {
  /**
   * Perform a "normal" checkout, analogous to `hg checkout` in Mercurial. Files
   * in the working copy will be changed to reflect the destination snapshot,
   * though files with conflicts will not be modified.
   */
  NORMAL = 0,

  /**
   * Do not checkout: exercise the checkout logic to discover potential
   * conflicts.
   */
  DRY_RUN = 1,

  /**
   * Perform a "forced" checkout, analogous to `hg checkout --clean` in
   * Mercurial. Conflicts between the working copy and destination snapshot will
   * be forcibly ignored in favor of the state of the new snapshot.
   */
  FORCE = 2,
}

enum ConflictType {
  /**
   * We failed to update this particular path due to an error
   */
  ERROR = 0,
  /**
   * A locally modified file was deleted in the new Tree
   */
  MODIFIED_REMOVED = 1,
  /**
   * An untracked local file exists in the new Tree
   */
  UNTRACKED_ADDED = 2,
  /**
   * The file was removed locally, but modified in the new Tree
   */
  REMOVED_MODIFIED = 3,
  /**
   * The file was removed locally, and also removed in the new Tree.
   */
  MISSING_REMOVED = 4,
  /**
   * A locally modified file was modified in the new Tree
   * This may be contents modifications, or a file type change (directory to
   * file or vice-versa), or permissions changes.
   */
  MODIFIED_MODIFIED = 5,
  /**
   * A directory was supposed to be removed or replaced with a file,
   * but it contains untracked files preventing us from updating it.
   */
  DIRECTORY_NOT_EMPTY = 6,
}

/**
 * Details about conflicts or errors that occurred during a checkout operation
 */
struct CheckoutConflict {
  1: PathString path;
  2: ConflictType type;
  3: string message;
}

struct ScmBlobMetadata {
  1: i64 size;
  2: BinaryHash contentsSha1;
}

struct ScmTreeEntry {
  1: binary name;
  2: i32 mode;
  3: ThriftObjectId id;
}

/*
 * Bits passed into debugInodeStatus to control the result set.
 * 0 is legacy behavior, equivalent to DIS_REQUIRE_LOADED | DIS_COMPUTE_BLOB_SIZES
 */

/**
 * No effect other than avoiding the legacy behavior.
 */
const i64 DIS_ENABLE_FLAGS = 1;

/**
 * Only return inodes currently loaded in memory.
 */
const i64 DIS_REQUIRE_LOADED = 2;

/**
 * Only return materialized inodes.
 */
const i64 DIS_REQUIRE_MATERIALIZED = 4;

/**
 * Return accurate blob sizes, which may require fetching blob metadata from
 * the backing store.
 */
const i64 DIS_COMPUTE_BLOB_SIZES = 8;

/**
 * Returns accurate mode_t bits, including ownership. When unset, only
 * the dtype bits are set.
 */
const i64 DIS_COMPUTE_ACCURATE_MODE = 16;

/**
 * Only return data for the passed in directory, do not attempt to recurse down
 * to its childrens.
 */
const i64 DIS_NOT_RECURSIVE = 32;

struct TreeInodeEntryDebugInfo {
  /**
   * The entry name.  This is just a PathComponent, not the full path
   */
  1: binary name;
  /**
   * The inode number, or 0 if no inode number has been assigned to
   * this entry
   */
  2: i64 inodeNumber;
  /**
   * The entry mode_t value
   */
  3: i32 mode;
  /**
   * True if an InodeBase object exists for this inode or not.
   */
  4: bool loaded;
  /**
   * True if an the inode is materialized in the overlay
   */
  5: bool materialized;
  /**
   * If materialized is false, hash contains the ID of the underlying source
   * control Blob or Tree.
   */
  6: ThriftObjectId hash;
  /**
   * Size of the file in bytes. It won't be set for directories.
   */
  7: optional i64 fileSize;
}

struct GetFetchedFilesResult {
  1: map<string, set<PathString>> fetchedFilePaths;
}

struct WorkingDirectoryParents {
  1: ThriftRootId parent1;
  // This field is never used by EdenFS.
  2: optional ThriftRootId parent2;
}

struct TreeInodeDebugInfo {
  1: i64 inodeNumber;
  2: binary path;
  3: bool materialized;
  // Traditionally, treeHash was a 20-byte binary hash, but now it's an
  // arbitrary-length human-readable string.
  4: ThriftObjectId treeHash;
  5: list<TreeInodeEntryDebugInfo> entries;
  6: i64 refcount;
}

struct InodePathDebugInfo {
  1: PathString path;
  2: bool loaded;
  3: bool linked;
}

/**
 * Where debug methods should fetch data from.
 */
enum DataFetchOrigin {
  NOWHERE = 0,
  ANYWHERE = 1,
  MEMORY_CACHE = 2,
  DISK_CACHE = 4,
  LOCAL_BACKING_STORE = 8,
  REMOTE_BACKING_STORE = 16,
/* NEXT_WHERE = 2^x */
} (cpp2.enum_type = 'uint64_t')

struct DebugGetScmBlobRequest {
  1: MountId mountId;
  # id of the blob we would like to fetch
  2: ThriftObjectId id;
  # where we should fetch the blob from
  3: DataFetchOriginSet origins; # DataFetchOrigin
}

union ScmBlobOrError {
  1: binary blob;
  2: EdenError error;
}

struct ScmBlobWithOrigin {
  # the blob data
  1: ScmBlobOrError blob;
  # where the blob was fetched from
  2: DataFetchOrigin origin;
}

struct DebugGetScmBlobResponse {
  1: list<ScmBlobWithOrigin> blobs;
}

struct DebugGetBlobMetadataRequest {
  1: MountId mountId;
  # id of the blob we would like to fetch metadata for
  2: ThriftObjectId id;
  # where we should fetch the blob metadata from
  3: DataFetchOriginSet origins; # DataFetchOrigin
}

union BlobMetadataOrError {
  1: ScmBlobMetadata metadata;
  2: EdenError error;
}

struct BlobMetadataWithOrigin {
  # the blob data
  1: BlobMetadataOrError metadata;
  # where the blob was fetched from
  2: DataFetchOrigin origin;
}

struct DebugGetBlobMetadataResponse {
  1: list<BlobMetadataWithOrigin> metadatas;
}

struct ActivityRecorderResult {
  // 0 if the operation has failed. For example,
  // fail to start recording due to file permission issue
  // or fail to stop recording due to no active subscriber.
  1: i64 unique;
  2: optional PathString path;
}

struct ListActivityRecordingsResult {
  1: list<ActivityRecorderResult> recordings;
}

struct SetLogLevelResult {
  1: bool categoryCreated;
}

struct JournalInfo {
  1: i64 entryCount;
  // The estimated memory used by the journal in bytes
  2: i64 memoryUsage;
  // The duration of the journal in seconds
  3: i64 durationSeconds;
}

/**
 * Struct to store Information about inodes in a mount point.
 */
struct MountInodeInfo {
  2: i64 unloadedInodeCount;
  4: i64 loadedFileCount;
  5: i64 loadedTreeCount;
}

struct CacheStats {
  1: i64 entryCount;
  2: i64 totalSizeInBytes;
  3: i64 hitCount;
  4: i64 missCount;
  5: i64 evictionCount;
  6: i64 dropCount;
}

/*
 * Bits that control the stats returned from  getStatInfo
 */
const i64 STATS_MOUNTS_STATS = 0x1;
const i64 STATS_COUNTERS = 0x2;
const i64 STATS_SMAPS = 0x4;
const i64 STATS_PRIVATE_BYTES = 0x8;
const i64 STATS_RSS_BYTES = 0x10;
const i64 STATS_CACHE_STATS = 0x20;
const i64 STATS_ALL = 0xFFFF;

/**
 * Struct to store fb303 counters from ServiceData.getCounters() and inode
 * information of all the mount points.
 */
struct InternalStats {
  /**
  * fbf303 counter of inodes unloaded by periodic job.
  * Populated if STATS_COUNTERS is set.
  */
  1: optional i64 periodicUnloadCount;
  /**
   * counters is the list of fb303 counters, key is the counter name, value is the
   * counter value.
   * Populated if STATS_COUNTERS is set.
   */
  2: optional map<string, i64> counters;
  /**
   * mountPointInfo is a map whose key is the path of the mount point and value
   * is the details like number of loaded inodes,unloaded inodes in that mount
   * and number of materialized inodes in that mountpoint.
   * Populated if STATS_MOUNTS_STATS is set.
   */
  3: optional map<PathString, MountInodeInfo> mountPointInfo;
  /**
   * Linux-only: the contents of /proc/self/smaps, to be parsed by the caller.
   * Populated if STATS_SMAPS is set.
   */
  4: optional binary smaps;
  /**
   * Linux-only: privateBytes populated from contents of /proc/self/smaps.
   * Populated with current value (the fb303 counters value is an average).
   * Populated if STATS_PRIVATE_BYTES is set.
   */
  5: optional i64 privateBytes;
  /**
   * Linux-only: vmRSS bytes is populated from contents of /proc/self/stats.
   * Populated with current value (the fb303 counters value is an average).
   * Populated if STATS_RSS_BYTES is set.
   */
  6: optional i64 vmRSSBytes;
  /**
   * Statistics about the in-memory blob cache.
   * Populated if STATS_CACHE_STATS is set.
   */
  7: optional CacheStats blobCacheStats;
  /**
   * mountPointJournalInfo is a map whose key is the path of the mount point
   * and whose value is information about the journal on that mount
   * Populated if STATS_MOUNTS_STATS is set.
   */
  8: optional map<PathString, JournalInfo> mountPointJournalInfo;
  /**
   * Statistics about the in-memory tree cache.
   * Populated if STATS_CACHE_STATS is set.
   */
  9: optional CacheStats treeCacheStats;
}

/**
 * Common timestamps for every trace event, used to measure durations and
 * display wall clock time.
 */
struct TraceEventTimes {
  // Nanoseconds since epoch.
  1: i64 timestamp;
  // Nanoseconds since arbitrary clock base, used for computing request
  // durations between start and finish.
  2: i64 monotonic_time_ns;
}

enum ThriftRequestEventType {
  UNKNOWN = 0,
  START = 1,
  FINISH = 2,
}

struct ThriftRequestEvent {
  1: TraceEventTimes times;
  2: ThriftRequestEventType eventType;
  3: ThriftRequestMetadata requestMetadata;
}

/**
 * Return value for the getRetroactiveThriftRequestEvents() function.
 */
struct GetRetroactiveThriftRequestEventsResult {
  1: list<ThriftRequestEvent> events;
}

struct RequestInfo {
  // The pid that originated this request.
  1: optional pid_t pid;
  // If available, the binary name corresponding to `pid`.
  2: optional string processName;
}

enum HgEventType {
  UNKNOWN = 0,
  QUEUE = 1,
  START = 2,
  FINISH = 3,
}

enum HgResourceType {
  UNKNOWN = 0,
  BLOB = 1,
  TREE = 2,
  BLOBMETA = 3,
}

enum HgImportPriority {
  LOW = 0,
  NORMAL = 1,
  HIGH = 2,
}

enum HgImportCause {
  UNKNOWN = 0,
  FS = 1,
  THRIFT = 2,
  PREFETCH = 3,
}

struct HgEvent {
  1: TraceEventTimes times;

  2: HgEventType eventType;
  3: HgResourceType resourceType;

  4: i64 unique;

  // HG manifest node ID as 40-character hex string.
  5: string manifestNodeId;
  6: binary path;

  7: optional RequestInfo requestInfo;
  8: HgImportPriority importPriority;
  9: HgImportCause importCause;
}

/**
 * Parameters for the getRetroactiveHgEvents() function.
 */
struct GetRetroactiveHgEventsParams {
  1: PathString mountPoint;
}

/**
 * Return value for the getRetroactiveHgEvents() function.
 */
struct GetRetroactiveHgEventsResult {
  1: list<HgEvent> events;
}

enum InodeType {
  TREE = 0,
  FILE = 1,
}

enum InodeEventType {
  UNKNOWN = 0,
  MATERIALIZE = 1,
  LOAD = 2,
}

enum InodeEventProgress {
  START = 0,
  END = 1,
  FAIL = 2,
}

struct InodeEvent {
  2: i64 ino;
  3: InodeType inodeType;
  4: InodeEventType eventType;
  // Duration is in microseconds (μs)
  5: i64 duration;
  6: InodeEventProgress progress;
  7: TraceEventTimes times;
  8: PathString path;
}

struct TaskEvent {
  1: TraceEventTimes times;
  2: string name;
  3: string threadName;
  4: i64 threadId;
  // Both duration and start are in microseconds (μs)
  5: unsigned64 duration;
  6: unsigned64 start;
}

/**
 * Parameters for the getRetroactiveInodeEvents() function.
 */
struct GetRetroactiveInodeEventsParams {
  1: PathString mountPoint;
}

/**
 * Return value for the getRetroactiveInodeEvents() function.
 */
struct GetRetroactiveInodeEventsResult {
  1: list<InodeEvent> events;
}

struct FuseCall {
  // This field is deprecated because its use is not worth the TraceBus
  // storage. It may be brought back in some other form.
  //1: i32 len;

  2: i32 opcode;
  // FUSE supplies a unique ID, but it is recycled so quickly that it's not
  // very useful. We report our own process-unique ID.
  3: i64 unique;
  4: i64 nodeid;
  5: i32 uid;
  6: i32 gid;
  7: pid_t pid;

  8: string opcodeName;
  9: optional string processName;
}

struct NfsCall {
  1: i32 xid;
  2: i32 procNumber;
  3: string procName;
}

enum PrjfsTraceCallType {
  INVALID = 0,
  /* Write operations */
  FILE_OPENED = 1,
  NEW_FILE_CREATED = 2,
  FILE_OVERWRITTEN = 3,
  PRE_DELETE = 4,
  PRE_RENAME = 5,
  PRE_SET_HARDLINK = 6,
  FILE_RENAMED = 7,
  HARDLINK_CREATED = 8,
  FILE_HANDLE_CLOSED_NO_MODIFICATION = 9,
  FILE_HANDLE_CLOSED_FILE_MODIFIED = 10,
  FILE_HANDLE_CLOSED_FILE_DELETED = 11,
  FILE_PRE_CONVERT_TO_FULL = 12,
  /* Read operations */
  START_ENUMERATION = 13,
  END_ENUMERATION = 14,
  GET_ENUMERATION_DATA = 15,
  GET_PLACEHOLDER_INFO = 16,
  QUERY_FILE_NAME = 17,
  GET_FILE_DATA = 18,
  CANCEL_COMMAND = 19,
}

struct PrjfsCall {
  1: i32 commandId;
  2: i32 pid;
  3: PrjfsTraceCallType callType;
}

/**
 * Metadata about an in-progress Thrift request.
 */
struct ThriftRequestMetadata {
  1: i64 requestId;
  2: string method;
  3: pid_t clientPid;
}

struct GetConfigParams {
  // Whether to reload the config from disk to make sure it is up-to-date
  1: eden_config.ConfigReloadBehavior reload = eden_config.ConfigReloadBehavior.AutoReload;
}

struct GetStatInfoParams {
  // a bitset that indicates the requested stats. 0 indicates all stats are requested.
  1: i64 statsMask;
}

struct DebugInvalidateRequest {
  1: MountId mount;
  // Relative path in the repo to recursively invalidate
  2: PathString path;
  // Files last accessed before now-age will be invalidated. A zero age means
  // all inodes.
  3: TimeSpec age;
  4: SyncBehavior sync;
  // Run the invalidation in the background.
  5: bool background;
}

struct DebugInvalidateResponse {
  // Number of files that were successfully invalidated. When a background
  // invalidation is requested, this will always be 0.
  1: unsigned64 numInvalidated;
}

/**
 * A representation of the system-dependent dirent::d_type field.
 * The bits and their interpretation is system dependent.
 * This value is u8 on all systems that implement it.  We
 * use i16 to pass this through thrift, which doesn't have unsigned
 * numbers
 */
typedef i16 OsDtype

enum SourceControlType {
  TREE = 0,
  REGULAR_FILE = 1,
  EXECUTABLE_FILE = 2,
  SYMLINK = 3,
  UNKNOWN = 4, // File types that can not be versioned in source control.
// Currently includes things like FIFOs and sockets.
}

/**
 * These numbers match up with Linux and macOS.
 * Windows doesn't have dtype_t, but a subset of these map to and from
 * the GetFileType and dwFileAttributes equivalents.
 *
 * Dtype and OsDtype can be cast between each other on all platforms.
 */
enum Dtype {
  UNKNOWN = 0,
  FIFO = 1, // DT_FIFO
  CHAR = 2, // DT_CHR
  DIR = 4, // DT_DIR
  BLOCK = 6, // DT_BLK
  REGULAR = 8, // DT_REG
  LINK = 10, // DT_LNK
  SOCKET = 12, // DT_SOCK
  WHITEOUT = 14, // DT_WHT
}

struct PredictiveFetch {
  // Number of directories to glob. If not specified, a default value (predictivePrefetchProfileSize in EdenConfig.h) is used.
  1: optional i32 numTopDirectories;
  // Fetch the most accessed diectories by user specified. If not specified, user is derived from the server state.
  2: optional string user;
  // Fetch the most accessed diectories in repository specified. If not specified, repo is derived from the mount.
  3: optional string repo;
  // Optional query parameter: fetch top most accessed directories with specified Operating System
  4: optional string os;
  // Optional query parameter: fetch top most accessed directories after startTime
  5: optional unsigned64 startTime;
  // Optional query parameter: fetch top most accessed directories before endTime
  6: optional unsigned64 endTime;
}

/** Params for prefetchFiles(). */
struct PrefetchParams {
  1: PathString mountPoint;
  2: list<string> globs;
  // If set, don't prefetch matching blobs. Only prefetch trees.
  3: bool directoriesOnly = false;
  // Commit hashes for the revisions against which the globs should be
  // evaluated, if this is empty then globFiles will fall back to using only
  // the current revision.
  4: list<ThriftRootId> revisions;
  // The directory from which the glob should be evaluated. Defaults to the
  // repository root.
  5: PathString searchRoot;
  // If set, will run the prefetch but will not wait for the result.
  6: bool background = false;
  // When set, the globs list must be empty and the globbing pattern will be obtained
  // from an online service.
  7: optional PredictiveFetch predictiveGlob;
}

/** Params for globFiles(). */
struct GlobParams {
  1: PathString mountPoint;
  2: list<string> globs;
  3: bool includeDotfiles;
  // if true, prefetch matching blobs
  4: bool prefetchFiles;
  // if true, don't populate matchingFiles in the Glob
  // results.  This only really makes sense with prefetchFiles.
  5: bool suppressFileList;
  6: bool wantDtype;
  // Commit hashes for the revisions against which the globs should be
  // evaluated, if this is empty then globFiles will fall back to using only
  // the current revision.
  // If eden moves away from commit hashes this may become the tree hash
  // for the root tree agains which this glob should be evaluated.
  // There should be no duplicates in this list. If there are then
  // there maybe duplicate machingFile and originHash pairs in the coresponding
  // output Glob.
  7: list<ThriftRootId> revisions;
  // This has no effect.
  8: bool prefetchMetadata = true;
  // The directory from which the glob should be evaluated. Defaults to the
  // repository root.
  9: PathString searchRoot;
  // If set, will run the prefetch but will not wait for the result.
  10: bool background = false;
  // When set, the globs list must be empty and the globbing pattern will be obtained
  // from an online service.
  11: optional PredictiveFetch predictiveGlob;
  // Normally the returned file list will contain both files and directories.
  // Some clients would like to see only lists of files, this option tells us
  // wether to filter or not.
  // Note for Eden developers: when this is false the matchingFiles fileList
  // will not actually be the list that is fed into the backing store to prefetch.
  12: bool listOnlyFiles = false;
  // Should this glob query also synchronize the working copy?
  13: SyncBehavior sync;
}

struct Glob {
  /**
   * matchingFiles can contain duplicate values and is not guaranteed to be
   * sorted. However, no duplicates may have the same originCommits (note this
   * is not true should the input GlobParams contain duplicate revisions) .
   */
  1: list<PathString> matchingFiles;
  2: list<OsDtype> dtypes;
  /**
   * Currently these are the commit hash for the commit to which this file
   * belongs. But should eden move away from commit hashes this may become
   * the tree hash of the root tree to which this file belongs.
   */
  3: list<binary> originHashes;
}

struct AccessCounts {
  1: i64 fsChannelTotal;
  2: i64 fsChannelReads;
  3: i64 fsChannelWrites;
  4: i64 fsChannelBackingStoreImports;
  5: i64 fsChannelDurationNs;
  6: i64 fsChannelMemoryCacheImports;
  7: i64 fsChannelDiskCacheImports;
}

struct MountAccesses {
  1: map<pid_t, AccessCounts> accessCountsByPid;
  2: map<pid_t, i64> fetchCountsByPid;
}

struct GetAccessCountsResult {
  1: map<pid_t, binary> cmdsByPid;
  2: map<PathString, MountAccesses> accessesByMount;
// TODO: Count the number of thrift requests
// 3: map<pid_t, AccessCount> thriftAccesses
}

enum TracePointEvent {
  // Start of a new block
  START = 0,
  // End of a block
  STOP = 1,
}

struct TracePoint {
  // Holds nanoseconds since the epoch
  1: i64 timestamp;
  // Opaque identifier for the entire trace - used to associate this
  // tracepoint with other tracepoints across an entire request
  2: i64 traceId;
  // Opaque identifier for this "block" where a block is some logical
  // piece of work with a well-defined start and stop point
  3: i64 blockId;
  // Opaque identifer for the parent block from which the current
  // block was constructed - used to create causal relationships
  // between blocks
  4: i64 parentBlockId;
  // The name of the block, only set on the tracepoint starting the
  // block, must point to a statically allocated cstring
  5: string name = "";
  // What event this trace point represents
  6: TracePointEvent event;
  // Thread ID of where this block started
  7: i64 threadId;
}

struct FaultDefinition {
  1: string keyClass;
  2: string keyValueRegex;
  // If count is non-zero this fault will automatically expire after it has
  // been hit count times.
  3: i64 count;
  // If block is true the fault will block until explicitly unblocked later.
  // delayMilliseconds and errorMessage will be ignored if block is true
  4: bool block;
  5: i64 delayMilliseconds;
  6: optional string errorType;
  7: optional string errorMessage;
  // If kill is true the fault will exit the process ungracefully.
  // block, delay, and errorMessage will be ignored if kill is true.
  8: bool kill;
}

struct RemoveFaultArg {
  1: string keyClass;
  2: string keyValueRegex;
}

struct UnblockFaultArg {
  1: optional string keyClass;
  2: optional string keyValueRegex;
  3: optional string errorType;
  4: optional string errorMessage;
}

struct GetScmStatusResult {
  1: ScmStatus status;
  // The version of the EdenFS daemon.
  // This is returned since we usually want status calls to be able to check
  // the current EdenFS version and warn the user if EdenFS is running an old
  // or known-bad version.
  2: string version;
}

struct GetScmStatusParams {
  /**
   * The Eden checkout to query
   */
  1: PathString mountPoint;

  /**
   * The commit ID of the current working directory parent commit.
   *
   * An error will be returned if this is not actually the current parent
   * commit.  This behavior exists to support callers that do not perform their
   * own external synchronization around access to the current parent commit,
   * like Mercurial.
   */
  2: ThriftRootId commit;

  /**
   * Whether ignored files should be reported in the results.
   *
   * Some special source-control related files (e.g., inside the .hg or .git
   * directory) will never be reported even when listIgnored is true.
   */
  3: bool listIgnored = false;
}

/**
  * BackingStore object type. Caller will response to verify the type of the content
  * matching the parameters passed. Exception will be thrown if type mismatch.
  */
enum ObjectType {
  TREE = 0,
  REGULAR_FILE = 1,
  EXECUTABLE_FILE = 2,
  SYMLINK = 3,
}

struct SetPathObjectIdObject {
  1: PathString path;
  2: ThriftObjectId objectId;
  3: ObjectType type;
}

// Any new use case should try to avoid using path, objectId and type, they will be deprecated.
// Please use objects instead which would batch requests. If both path/objectId/type and objects
// both present, the singlular will be added to the objects.
struct SetPathObjectIdParams {
  1: PathString mountPoint;
  // TODO: deprecate path, objectId and type
  2: PathString path;
  3: ThriftObjectId objectId;
  4: ObjectType type;
  5: CheckoutMode mode;
  // Extra request infomation. i.e. build uuid, cache session id.
  6: optional map<string, string> requestInfo;
  7: list<SetPathObjectIdObject> objects;
}

struct SetPathObjectIdResult {
  1: list<CheckoutConflict> conflicts;
}

struct CheckOutRevisionParams {
  /**
   * The hg root manifest that corresponds to the commit (if known).
   *
   * When a commit is newly created, EdenFS won't know the commit
   * to root-manifest mapping for the commit, and won't be able to find
   * out from the import helper until the import helper re-opens the
   * repo.  To speed this up, Mercurial clients may optionally provide
   * the hash of the root manifest directly, so that EdenFS doesn't
   * need to look it up.
   */
  1: optional BinaryHash hgRootManifest;
}

struct ResetParentCommitsParams {
  /**
   * The hg root manifest that corresponds to the commit (if known).
   *
   * When a commit is newly created, EdenFS won't know the commit
   * to root-manifest mapping for the commit, and won't be able to find
   * out from the import helper until the import helper re-opens the
   * repo.  To speed this up, Mercurial clients may optionally provide
   * the hash of the root manifest directly, so that EdenFS doesn't
   * need to look it up.
   */
  1: optional BinaryHash hgRootManifest;
}

struct RemoveRecursivelyParams {
  1: PathString mountPoint;
  2: PathString path;
  3: SyncBehavior sync;
}

struct SynchronizeWorkingCopyParams {
  1: SyncBehavior sync;
}

struct EnsureMaterializedParams {
  1: PathString mountPoint;
  2: list<PathString> paths;
  // Materialize on the background and do not block.
  3: bool background = false;
  // Also materialize symlink target if the target is also in the same mount
  4: bool followSymlink = false;
  5: SyncBehavior sync;
}

struct MatchFilesystemPathResult {
  1: optional EdenError error;
}

struct MatchFileSystemResponse {
  1: list<MatchFilesystemPathResult> results;
}

struct MatchFileSystemRequest {
  1: MountId mountPoint;
  2: list<PathString> paths;
}

struct ChangeOwnershipRequest {
  1: PathString mountPoint;
  2: i64 uid;
  3: i64 gid;
}

struct ChangeOwnershipResponse {}

service EdenService extends fb303_core.BaseService {
  list<MountInfo> listMounts() throws (1: EdenError ex);
  void mount(1: MountArgument info) throws (1: EdenError ex);
  void unmount(1: PathString mountPoint) throws (1: EdenError ex);

  /**
   * Potentially check out the specified snapshot, reporting conflicts (and
   * possibly errors), as appropriate.
   *
   * If the checkoutMode is FORCE, the working directory will be forcibly
   * updated to the contents of the new snapshot, even if there were conflicts.
   * Conflicts will still be reported in the return value, but the files will be
   * updated to their new state.
   *
   * If the checkoutMode is NORMAL, files with conflicts will be left
   * unmodified. Files that are untracked in both the source and destination
   * snapshots are always left unchanged, even if force is true.
   *
   * If the checkoutMode is DRY_RUN, then no files are modified in the working
   * copy and the current snapshot does not change. However, potential conflicts
   * are still reported in the return value.
   *
   * On successful return from this function (unless it is a DRY_RUN), the mount
   * point will point to the new snapshot, even if some paths had conflicts or
   * errors. The caller is responsible for taking appropriate action to update
   * these paths as desired after checkOutRevision() returns.
   *
   * Note: this internally synchronize the working copy.
   */
  list<CheckoutConflict> checkOutRevision(
    1: PathString mountPoint,
    2: ThriftRootId snapshotHash,
    3: CheckoutMode checkoutMode,
    4: CheckOutRevisionParams params,
  ) throws (1: EdenError ex);

  /**
   * Reset the working directory's parent commits, without changing the working
   * directory contents.
   *
   * This operation is equivalent to `git reset --soft` or `hg reset --keep`
   */
  void resetParentCommits(
    1: PathString mountPoint,
    2: WorkingDirectoryParents parents,
    3: ResetParentCommitsParams params,
  ) throws (1: EdenError ex);

  /**
   * Ensure that all inflight working copy modification have completed.
   *
   * On some platforms, EdenFS is processing working copy modifications
   * callbacks from the platform in an asynchronous manner, which means that by
   * the time a write/creat/mkdir/unlink/etc syscall returns from the kernel,
   * EdenFS may not have updated its internal state.
   *
   * Thus, an application making changes to the working copy and quickly
   * requesting EdenFS to perform an operation on it will race with EdenFS
   * updating its internal state and may thus get stale data.
   *
   * To avoid this, applications can call this method prior to issuing other
   * Thrift requests (such as getSHA1, globFiles, etc) to wait for EdenFS to
   * update its internal state. Applications that care about synchronizing
   * EdenFS up to a certain point in time are expected to call this once and
   * then issue all their thrift requests without synchronizing.
   *
   * As an alternative, applications may also set the SyncBehavior of a Thrift
   * method to a non-zero value to achieve the same result.
   *
   * Some Thrift methods are implicitely synchronizing, their documentation
   * will state it.
   */
  void synchronizeWorkingCopy(
    1: PathString mountPoint,
    2: SynchronizeWorkingCopyParams params,
  ) throws (1: EdenError ex);

  /**
   * For each path, returns an EdenError instead of the SHA-1 if any of the
   * following occur:
   * - path is the empty string.
   * - path identifies a non-existent file.
   * - path identifies something that is not an ordinary file (e.g., symlink
   *   or directory).
   *
   * Note: may return stale data if synchronizeWorkingCopy isn't called, and if
   * the SyncBehavior specify a 0 timeout. see the documentation for both of
   * these for more details.
   */
  list<SHA1Result> getSHA1(
    1: PathString mountPoint,
    2: list<PathString> paths,
    3: SyncBehavior sync,
  ) throws (1: EdenError ex);

  /**
   * For each path, returns an EdenError instead of the BLAKE3 if any of the
   * following occur:
   * - path is the empty string.
   * - path identifies a non-existent file.
   * - path identifies something that is not an ordinary file (e.g., symlink
   *   or directory).
   *
   * Note: may return stale data if synchronizeWorkingCopy isn't called, and if
   * the SyncBehavior specify a 0 timeout. see the documentation for both of
   * these for more details.
   */
  list<Blake3Result> getBlake3(
    1: PathString mountPoint,
    2: list<PathString> paths,
    3: SyncBehavior sync,
  ) throws (1: EdenError ex);

  /**
   * On systems that support bind mounts, establish a bind mount within the
   * repo such that `mountPoint / repoPath` is redirected to `targetPath`.
   * If `repoPath` is already a bind mount managed by eden, this function
   * will throw an error.
   * If `repoPath` is not a directory then it will be created similar to
   * running `mkdir -p mountPoint/repoPath` and then the bind mount
   * will be established.
   * If `repoPath` exists and is not a directory, an error will be thrown.
   * If the bind mount cannot be set up, an error will be thrown.
   */
  void addBindMount(
    1: PathString mountPoint,
    2: PathString repoPath,
    3: PathString targetPath,
  ) throws (1: EdenError ex);

  /**
   * Removes the bind mount specified by `repoPath` from the set of managed
   * bind mounts.
   * If `repoPath` is not a bind mount managed by eden, this function
   * will throw an error.
   * If the bind mount cannot be removed, an error will be thrown.
   */
  void removeBindMount(
    1: PathString mountPoint,
    2: PathString repoPath,
  ) throws (1: EdenError ex);

  /** Returns the sequence position at the time the method is called.
   * Returns the instantaneous value of the journal sequence number.
   */
  JournalPosition getCurrentJournalPosition(1: PathString mountPoint) throws (
    1: EdenError ex,
  );

  /** Returns the set of files (and dirs) that changed since a prior point.
   * If fromPosition.mountGeneration is mismatched with the current
   * mountGeneration, throws an EdenError with errorCode = ERANGE.
   * If the domain required by fromPosition goes past the Journal's memory,
   * throws an EdenError with errorCode = EDOM.
   * This indicates that eden cannot compute the delta for the requested
   * range.  The client will need to recompute a new baseline using
   * other available functions in EdenService.
   */
  FileDelta getFilesChangedSince(
    1: PathString mountPoint,
    2: JournalPosition fromPosition,
  ) throws (1: EdenError ex);

  /** Sets the memory limit on the journal such that the journal will forget
   * old data to keep itself under a certain estimated memory use.
   */
  void setJournalMemoryLimit(1: PathString mountPoint, 2: i64 limit) throws (
    1: EdenError ex,
  );

  /** Gets the memory limit on the journal
   */
  i64 getJournalMemoryLimit(1: PathString mountPoint) throws (1: EdenError ex);

  /** Forces the journal to flush, sending a truncated result to subscribers
   */
  void flushJournal(1: PathString mountPoint) throws (1: EdenError ex);

  /**
   * Returns the journal entries for the specified params. Useful for auditing
   * the changes that Eden has sent to Watchman. Note that the most recent
   * journal entries will be at the front of the list in
   * DebugGetRawJournalResponse.
   */
  DebugGetRawJournalResponse debugGetRawJournal(
    1: DebugGetRawJournalParams params,
  ) throws (1: EdenError ex);

  /**
   * Returns the subset of information about a list of paths that can
   * be determined from each's parent directory tree. For now, that
   * includes whether the entry exists and its dtype.
   *
   * Note: may return stale data if synchronizeWorkingCopy isn't called, and if
   * the SyncBehavior specify a 0 timeout. see the documentation for both of
   * these for more details.
   */
  list<EntryInformationOrError> getEntryInformation(
    1: PathString mountPoint,
    2: list<PathString> paths,
    3: SyncBehavior sync,
  ) throws (1: EdenError ex);

  /**
   * Returns a subset of the stat() information for a list of paths.
   * The returned list of information corresponds to the input list of
   * paths; eg; result[0] holds the information for paths[0].
   * We only support returning the instantaneous information about
   * these paths, as we cannot answer with historical information about
   * files in the overlay.
   *
   * Note: may return stale data if synchronizeWorkingCopy isn't called, and if
   * the SyncBehavior specify a 0 timeout. see the documentation for both of
   * these for more details.
   */
  list<FileInformationOrError> getFileInformation(
    1: PathString mountPoint,
    2: list<PathString> paths,
    3: SyncBehavior sync,
  ) throws (1: EdenError ex);

  /**
   * Returns the requested file attributes for the provided list of files.
   * The result maps the files to attribute results which may be an EdenError
   * or a FileAttributeDataV2 struct.
   *
   * Unlike the getAttributesFromFiles endpoint, this does not assume that all
   * the inputs are regular files. This endpoint will attempt to return
   * attributes for any type of file (directory included). Note that some
   * attributes are not currently supported, like sha1 and size for directories
   * and symlinks. At some point EdenFS may be able to support such attributes.
   *
   * Note: may return stale data if synchronizeWorkingCopy isn't called, and if
   * the SyncBehavior specifies a 0 timeout. See the documentation for both of
   * these for more details.
   */
  GetAttributesFromFilesResultV2 getAttributesFromFilesV2(
    1: GetAttributesFromFilesParams params,
  ) throws (1: EdenError ex);

  /**
   * DEPRECATED - prefer getAttributesFromFilesV2.
   *
   * Returns the requested file attributes for the provided list of files.
   *
   * This API assumes all the given paths corespond to regular files.
   * We return EdenErrors instead of attributes for paths that correspond to
   * directories or symlinks.
   *
   * Note: may return stale data if synchronizeWorkingCopy isn't called, and if
   * the SyncBehavior specify a 0 timeout. see the documentation for both of
   * these for more details.
   */
  GetAttributesFromFilesResult getAttributesFromFiles(
    1: GetAttributesFromFilesParams params,
  ) throws (1: EdenError ex);

  /**
   * Returns the requested file attributes for each of the entries in the
   * specified directories. . and .. are not included in the entry list.
   *
   * sha1 and size are not available for directories and symlinks, error values
   * will be returned for those attributes for entries of those types.
   *
   * Note: may return stale data if synchronizeWorkingCopy isn't called, and if
   * the SyncBehavior specify a 0 timeout. see the documentation for both of
   * these for more details.
   */
  ReaddirResult readdir(1: ReaddirParams params) throws (1: EdenError ex);

  /**
   * DEPRECATED: Use globFiles().
   *
   * Note: may return stale data if synchronizeWorkingCopy isn't called, see the
   * documentation for that method.
   */
  list<PathString> glob(
    1: PathString mountPoint,
    2: list<string> globs,
  ) throws (1: EdenError ex);

  /**
   * Returns a list of files that match the GlobParams, notably,
   * the list of glob patterns.
   * There are no duplicate values in the result.
   *
   * Note: may return stale data if synchronizeWorkingCopy isn't called, and if
   * the SyncBehavior specify a 0 timeout. see the documentation for both of
   * these for more details.
   */
  Glob globFiles(1: GlobParams params) throws (1: EdenError ex);

  /**
   * Has the same behavior as globFiles, but should be called in the case of a prefetch.
   * This request could be deprioritized since it will be assumed that this call is used
   * for optimization and the result not relied on for operations. This command does not
   * return the list of prefetched files.
   */
  void prefetchFiles(1: PrefetchParams params) throws (1: EdenError ex) (
    priority = 'BEST_EFFORT',
  );

  /**
   * Gets a list of a user's most accessed directories, performs
   * prefetching as specified by PredictiveGlobParams, and returns
   * a list of files matching the glob patterns.
   * There are no duplicate values in the result.
   *
   * Note: may return stale data if synchronizeWorkingCopy isn't called, and if
   * the SyncBehavior specify a 0 timeout. see the documentation for both of
   * these for more details.
   */
  Glob predictiveGlobFiles(1: GlobParams params) throws (1: EdenError ex);

  /**
   * Chowns all files in the requested mount to the requested uid and gid
   *
   * DEPRECATED: Prefer using ChangeOwnership in new code.  Callers may still
   * need to fall back to chown() if talking to an older edenfs daemon
   * that does not support ChangeOwnership() yet.
   */
  void chown(1: PathString mountPoint, 2: i32 uid, 3: i32 gid);

  /**
   * Changes ownership all files in the requested mount to the requested uid and gid
   */
  ChangeOwnershipResponse changeOwnership(
    1: ChangeOwnershipRequest request,
  ) throws (1: EdenError ex);

  /**
   * Return the list of files that are different from the specified source
   * control commit.
   *
   * Note: this internally synchronize the working copy and thus the returned
   * data is guaranteed to return be the set of files that changed prior to
   * calling this method.
   */
  GetScmStatusResult getScmStatusV2(1: GetScmStatusParams params) throws (
    1: EdenError ex,
  );

  /**
   * Get the status of the working directory against the specified commit.
   *
   * Note: this internally synchronize the working copy and thus the returned
   * data is guaranteed to return be the set of files that changed prior to
   * calling this method.
   *
   * DEPRECATED: Prefer using getScmStatusV2() in new code.  Callers may still
   * need to fall back to getScmStatus() if talking to an older edenfs daemon
   * that does not support getScmStatusV2() yet.
   */
  ScmStatus getScmStatus(
    1: PathString mountPoint,
    2: bool listIgnored,
    3: ThriftRootId commit,
  ) throws (1: EdenError ex);

  /**
   * Computes the status between two specified revisions.
   * This does not care about the state of the working copy.
   *
   * This is used by Watchman and which will be deprecated once EdenFS provides
   * an API to get the set of files changed between 2 journal clocks.
   */
  ScmStatus getScmStatusBetweenRevisions(
    1: PathString mountPoint,
    2: ThriftRootId oldHash,
    3: ThriftRootId newHash,
  ) throws (1: EdenError ex);

  /**
   * When eden doctor or another tool noticies that EdenFS is out of sync with
   * the filesystem this API can be used to poke EdenFS into noticing the file
   * change.
   */
  MatchFileSystemResponse matchFilesystem(
    1: MatchFileSystemRequest params,
  ) throws (1: EdenError ex);

  //////// Administrative APIs ////////

  /**
   * Returns information about the running process, including pid and command
   * line.
   */
  DaemonInfo getDaemonInfo() throws (1: EdenError ex) (priority = 'IMPORTANT');

  /**
  * Returns information about the privhelper process, including accesibility.
  */
  PrivHelperInfo checkPrivHelper() throws (1: EdenError ex);

  /**
   * DEPRECATED
   *
   * Returns the pid of the running edenfs daemon. New code should call
   * getDaemonInfo instead. This method exists for Thrift clients that
   * predate getDaemonInfo, such as older versions of the CLI.
   */
  i64 getPid() throws (1: EdenError ex);

  /**
   * Ask the server to shutdown and provide it some context for its logs
   */
  void initiateShutdown(1: string reason) throws (1: EdenError ex);

  /**
   * Get the current configuration settings
   */
  eden_config.EdenConfigData getConfig(1: GetConfigParams params) throws (
    1: EdenError ex,
  );

  /**
   * Ask eden to reload its configuration data from disk.
   */
  void reloadConfig() throws (1: EdenError ex);

  //////// Debugging APIs ////////

  /**
   * Get the contents of a source control Tree.
   *
   * This can be used to confirm if eden's LocalStore contains information
   * for the tree, and that the information is correct.
   *
   * If localStoreOnly is true, the data is loaded directly from the
   * LocalStore, and an error will be raised if it is not already present in
   * the LocalStore.  If localStoreOnly is false, the data may be retrieved
   * from the BackingStore if it is not already present in the LocalStore.
   */
  list<ScmTreeEntry> debugGetScmTree(
    1: PathString mountPoint,
    2: ThriftObjectId id,
    3: bool localStoreOnly,
  ) throws (1: EdenError ex);

  /**
   * Get the contents of a source control Blob.
   *
   * The origins field can control where to check for the blob. This will
   * attempt to fetch the blob from all locations and return the blob contents
   * from each of the locations.
   */
  DebugGetScmBlobResponse debugGetBlob(
    1: DebugGetScmBlobRequest request,
  ) throws (1: EdenError ex);

  /**
   * Get the metadata about a source control Blob.
   *
   * This retrieves the metadata about a source control Blob.  This returns
   * the size and contents SHA1 of the blob, which eden stores separately from
   * the blob itself.  This can also be a useful alternative to
   * debugGetBlob() when getting data about extremely large blobs.
   *
   * The origins field can control where to check for the blob. This will
   * attempt to fetch the blob from all locations and return the blob contents
   * from each of the locations.
   */
  DebugGetBlobMetadataResponse debugGetBlobMetadata(
    1: DebugGetBlobMetadataRequest request,
  ) throws (1: EdenError ex);

  /**
   * Get status about inodes with allocated inode numbers.
   *
   * This returns details about all previously-observed inode objects under the
   * given path.
   *
   * If the path argument is the empty string data will be returned about all
   * inodes in the entire mount point.  Otherwise the path argument should
   * refer to a subdirectory, and data will be returned for all inodes under
   * the specified subdirectory.
   *
   * The rename lock is not held while gathering this information, so the path
   * name information returned may not always be internally consistent.  If
   * renames were taking place while gathering the data, some inodes may show
   * up under multiple parents.  It's also possible that we may miss some
   * inodes during the tree walk if they were renamed from a directory that was
   * not yet walked into a directory that has already been walked.
   *
   * This API cannot return data about inodes that have been unlinked but still
   * have outstanding references.
   *
   * Note: may return stale data if synchronizeWorkingCopy isn't called, and if
   * the SyncBehavior specify a 0 timeout. see the documentation for both of
   * these for more details.
   */
  list<TreeInodeDebugInfo> debugInodeStatus(
    1: PathString mountPoint,
    2: PathString path,
    3: i64 flags,
    4: SyncBehavior sync,
  ) throws (1: EdenError ex);

  /**
   * Get the list of outstanding fuse requests
   *
   * This will return the list of FuseCall structure containing the data from
   * fuse_in_header.
   */
  list<FuseCall> debugOutstandingFuseCalls(1: PathString mountPoint);

  /**
   * Get the list of outstanding NFS requests
   *
   * This will return the list of NfsCall structure containing the data from the RPC request.
   */
  list<NfsCall> debugOutstandingNfsCalls(1: PathString mountPoint);

  /**
   * Get the list of outstanding ProjectedFS requests
   *
   * This will return the list of PrjfsCall structure containing the data from
   * the PRJ_CALLBACK_DATA.
   */
  list<PrjfsCall> debugOutstandingPrjfsCalls(1: PathString mountPoint);

  /**
   * Get the list of outstanding Thrift requests
   */
  list<ThriftRequestMetadata> debugOutstandingThriftRequests();

  /**
   * Start recording performance metrics such as files read
   *
   * This will return a structure containing unique id identifying this recording.
   */
  ActivityRecorderResult debugStartRecordingActivity(
    1: PathString mountPoint,
    2: PathString outputDir,
  );

  /**
   * Stop the recording identified by unique
   *
   * This will return a structure containing unique id identifying this recording
   * and, if the recording is successfully stopped, the output file path.
   */
  ActivityRecorderResult debugStopRecordingActivity(
    1: PathString mountPoint,
    2: i64 unique,
  );

  /**
   * Get the list of ongoing activity recordings
   *
   * This will return the list of ActivityRecorderResult structure
   * containing the id and output file path.
   */
  ListActivityRecordingsResult debugListActivityRecordings(
    1: PathString mountPoint,
  );

  /**
   * Get the InodePathDebugInfo for the inode that corresponds to the given
   * inode number. This provides the path for the inode and also indicates
   * whether the inode is currently loaded or not. Requires that the Eden
   * mountPoint be specified.
   */
  InodePathDebugInfo debugGetInodePath(
    1: PathString mountPoint,
    2: i64 inodeNumber,
  ) throws (1: EdenError ex);

  /**
   * Clear pidFetchCounts_ in ObjectStore to start a new recording of process
   * fetch counts.
   */
  void clearFetchCounts() throws (1: EdenError ex);
  void clearFetchCountsByMount(1: PathString mountPath) throws (
    1: EdenError ex,
  );

  /**
   * Queries all of the live Eden mounts for the processes that accessed FUSE
   * over the last `duration` seconds.
   *
   * Note that eden only maintains a few seconds worth of accesses.
   */
  GetAccessCountsResult getAccessCounts(1: i64 duration) throws (
    1: EdenError ex,
  );

  /**
   * Start recording paths of the files fetched from the backing store.
   *
   * Note that using this call twice will not clear the data and start a new
   * recording.
   */
  void startRecordingBackingStoreFetch() throws (1: EdenError ex);

  /**
   * Stop recording paths of the files fetched from the backing store.
   *
   * Note that using this call will return and clear the previously
   * collected data.
   */
  GetFetchedFilesResult stopRecordingBackingStoreFetch() throws (
    1: EdenError ex,
  );

  /**
   * Column by column, clears and compacts the LocalStore. All columns are
   * compacted, but only columns that contain ephemeral data are cleared.
   *
   * Even though the behavior of this method is identical to
   * debugClearLocalStoreCaches followed by debugCompactLocalStorage(), it is
   * separate so it can clear and compact each column in order to minimize the
   * risk of running out of disk space. Since RocksDB is a write-ahead logging
   * database, clearing a column increases its disk usage until it's compacted.
   */
  void clearAndCompactLocalStore() throws (1: EdenError ex);

  /**
   * Clears all data from the LocalStore that can be populated from the upstream
   * backing store.
   */
  void debugClearLocalStoreCaches() throws (1: EdenError ex);

  /**
   * Asks RocksDB to perform a compaction.
   */
  void debugCompactLocalStorage() throws (1: EdenError ex);

  /**
   * Requests EdenFS to drop all pending backing store fetches.
   * Returns the number of requests EdenFS dropped from the queue.
   */
  i64 debugDropAllPendingRequests() throws (1: EdenError ex);

  /**
  * Unloads unused Inodes from a directory inside a mountPoint whose last
  * access time is older than the specified age.
  *
  * The age parameter is a relative time to be subtracted from the current
  * (wall clock) time.
  */
  i64 unloadInodeForPath(
    1: PathString mountPoint,
    2: PathString path,
    3: TimeSpec age,
  ) throws (1: EdenError ex);

  /**
   * Garbage collect the repository by invalidating all the files/directories
   * below the passed in path who haven't been accessed since the given age.
   *
   * TODO: Rewrite as a streaming API once eden doctor is in Rust.
   */
  DebugInvalidateResponse debugInvalidateNonMaterialized(
    1: DebugInvalidateRequest params,
  ) throws (1: EdenError ex);

  /**
   * Flush all thread-local stats to the main ServiceData object.
   *
   * Thread-local counters are normally flushed to the main ServiceData once
   * a second.  flushStatsNow() can be used to flush thread-local counters on
   * demand, in addition to the normal once-a-second flush.
   *
   * This is mainly useful for unit and integration tests that want to ensure
   * they see up-to-date counter information without waiting for the normal
   * flush interval.
   */
  void flushStatsNow() throws (1: EdenError ex);

  /**
  * Invalidate kernel cache for inode.
  */
  void invalidateKernelInodeCache(
    1: PathString mountPoint,
    2: PathString path,
  ) throws (1: EdenError ex);

  /**
   * Gets the number of inodes unloaded by periodic job on an EdenMount.
   */
  InternalStats getStatInfo(1: GetStatInfoParams params) throws (
    1: EdenError ex,
  );

  void enableTracing();
  void disableTracing();
  list<TracePoint> getTracePoints();

  /**
   * Gets a list of thrift request events stored on the thrift server's ActivityBuffer.
   * Used for retroactive debugging by the `eden trace thrift --retroactive` command.
   */
  GetRetroactiveThriftRequestEventsResult getRetroactiveThriftRequestEvents() throws (
    1: EdenError ex,
  );

  /**
   * Gets a list of hg events stored in Eden's Hg ActivityBuffer. Used for
   * retroactive debugging by the `eden trace hg --retroactive` command.
   */
  GetRetroactiveHgEventsResult getRetroactiveHgEvents(
    1: GetRetroactiveHgEventsParams params,
  ) throws (1: EdenError ex);

  /**
   * Gets a list of inode events stored in a specified EdenMount's
   * ActivityBuffer. Used for retroactive debugging by the `eden trace inode
   * --retroactive` command. Supports inode load and materialization events.
   */
  GetRetroactiveInodeEventsResult getRetroactiveInodeEvents(
    1: GetRetroactiveInodeEventsParams params,
  ) throws (1: EdenError ex);

  /**
   * Configure a new fault in Eden's fault injection framework.
   *
   * This throws an exception if the fault injection framework was not enabled
   * when edenfs was started.
   */
  void injectFault(1: FaultDefinition fault) throws (1: EdenError ex);

  /**
   * Remove a fault previously defined with injectFault()
   *
   * Returns true if a matching fault was found and remove, and false
   * if no matching fault was found.
   */
  bool removeFault(1: RemoveFaultArg fault) throws (1: EdenError ex);

  /**
   * Unblock fault injection checks pending on a block fault.
   *
   * Returns the number of pending calls that were unblocked
   */
  i64 unblockFault(1: UnblockFaultArg info) throws (1: EdenError ex);

  /**
   * Directly load a BackingStore object identified by id at the given path.
   *
   * If any file or directory name conflict, the behavior is same with Checkout
   * This method is thread safe.
   */
  SetPathObjectIdResult setPathObjectId(
    1: SetPathObjectIdParams params,
  ) throws (1: EdenError ex);

  /**
   * Functionally same as rm -r command but more efficient since rm command would
   * load every subtree before unlinking it.
   */
  void removeRecursively(1: RemoveRecursivelyParams params) throws (
    1: EdenError ex,
  );

  /**
   * Eagerly materialize a list of paths, which can improve the latency of random reads.
   * If the path is a file, materialize the file.
   * If the path is a directory, recursively materialize its children.
   * If the path is a symlink, materialize its target if followSymlink option is set.
   *
   * This method should be used carefully, as EdenFS will copy the file contents into the overlay,
   * consuming disk space. Also, materialized files slow down checkout and status operations.
   */
  void ensureMaterialized(1: EnsureMaterializedParams params) throws (
    1: EdenError ex,
  );
}
