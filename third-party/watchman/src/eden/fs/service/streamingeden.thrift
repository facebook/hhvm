/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This software may be used and distributed according to the terms of the
 * GNU General Public License version 2.
 */

include "eden/fs/service/eden.thrift"
namespace cpp2 facebook.eden
namespace py3 eden.fs.service

enum FsEventType {
  UNKNOWN = 0,
  START = 1,
  FINISH = 2,
}

struct FsEvent {
  // Nanoseconds since epoch.
  1: i64 timestamp;
  // Nanoseconds since arbitrary clock base, used for computing request
  // durations between start and finish.
  2: i64 monotonic_time_ns;

  7: eden.TraceEventTimes times;

  3: FsEventType type;

  // See fuseRequest or prjfsRequest for the request opcode name.
  4: string arguments;

  // At most one of the *Request fields will be set, depending on the filesystem implementation.
  5: optional eden.FuseCall fuseRequest;
  10: optional eden.NfsCall nfsRequest;
  11: optional eden.PrjfsCall prjfsRequest;

  8: eden.RequestInfo requestInfo;

  /**
   * The result code sent back to the kernel.
   *
   * Positive is success, and, depending on the operation, may contain a nonzero result.
   *
   * If a FUSE request returns an inode which the kernel will reference, this field contains that inode numebr, so it can be correlated with future FUSE requests to that inode.
   * field is set. This field can be used to link the lookup/create/mknod
   * request to future FUSE requests on that inode.
   *
   * Negative indicates an error.
   */
  9: optional i64 result;
}

/*
 * Bits that control the events traced from traceFsEvents.
 *
 * edenfs internally categorizes FUSE requests as read, write, or other. That
 * is subject to change, and additional filtering bits may be added in the
 * future.
 */

const i64 FS_EVENT_READ = 1;
const i64 FS_EVENT_WRITE = 2;
const i64 FS_EVENT_OTHER = 4;

/**
 * The value of a stream item.
 *
 * Each stream item refers to a single file, along with the file status and its
 * type.
 */
struct ChangedFileResult {
  1: eden.PathString name;
  2: eden.ScmFileStatus status;
  // Dtype for this file which may be set to UNKNOWN.
  3: eden.Dtype dtype;
}

/**
 * Return value of the streamChangesSince.
 */
struct ChangesSinceResult {
  1: eden.JournalPosition toPosition;
}

/**
 * Argument to streamChangesSince API.
 */
struct StreamChangesSinceParams {
  1: eden.PathString mountPoint;
  2: eden.JournalPosition fromPosition;
}

struct TraceTaskEventsRequest {}

typedef binary EdenStartStatusUpdate

/**
 * This Thrift service defines streaming functions. It is separate from
 * EdenService because older Thrift runtimes do not support Thrift streaming,
 * primarily javadeprecated which is used by Buck. When Buck is updated to
 * use java-swift instead, we can merge EdenService and StreamingEdenService.
 */
service StreamingEdenService extends eden.EdenService {
  /**
   * Request notification about changes to the journal for
   * the specified mountPoint.
   *
   * IMPORTANT: Do not use the JournalPosition values in the stream. They are
   * meaningless. Instead, call getFilesChangedSince or
   * getCurrentJournalPosition which will return up-to-date information and
   * unblock future notifications on this subscription. If the subscriber
   * never calls getFilesChangedSince or getCurrentJournalPosition in
   * response to a notification on this stream, future notifications may not
   * arrive.
   *
   * This is an implementation of the subscribe API using the
   * new rsocket based streaming thrift protocol.
   * The name is temporary: we want to make some API changes
   * but want to start pushing out an implementation now because
   * we've seen inflated memory usage for the older `subscribe`
   * method above.
   */
  stream<eden.JournalPosition> subscribeStreamTemporary(
    1: eden.PathString mountPoint,
  );

  /**
   * Returns, in order, a stream of FUSE or PrjFS requests and responses for
   * the given mount.
   *
   * eventCategoryMask is a bitset which indicates the requested trace events.
   * 0 indicates all events are requested.
   */
  stream<FsEvent> traceFsEvents(
    1: eden.PathString mountPoint,
    2: i64 eventCategoryMask,
  );

  /**
   * Returns, in order, a stream of Thrift requests for the given mount.
   *
   * Each request has a unique ID and metadata including the thrift method
   * and calling process PID. Events are streamed for start and end events.
   */
  stream<eden.ThriftRequestEvent> traceThriftRequestEvents();

  /**
   * Returns, in order, a stream of hg import requests for the given mount.
   *
   * Each request has a unique ID and transitions through three states: queued,
   * started, and finished.
   */
  stream<eden.HgEvent> traceHgEvents(1: eden.PathString mountPoint);

  /**
   * Returns, in order, a stream of inode events for the given mount.
   *
   * This includes start and end events for Inode Materializations and Loads
   */
  stream<eden.InodeEvent> traceInodeEvents(1: eden.PathString mountPoint);

  /**
   * (Debugging only) This returns a stream of events when they are
   * finished processing.
   */
  stream<eden.TaskEvent> traceTaskEvents(1: TraceTaskEventsRequest request);

  /**
   * Returns a stream of changes since the given JournalPosition.
   *
   * Files are returned in no special order and aren't guaranteed to be unique.
   * For instance, a checkout from A to B and then back to A may return all the
   * files changed in between these revisions twice, once for the first
   * transition, a second time for the second transition.
   *
   * Since the stream can potentially contain a lot of files, clients are
   * advised to implement some bounding mechanism and close the stream when too
   * many files have been received.
   *
   * Along with the stream, this returns a ChangesSinceResult containing a
   * JournalPosition to inform the client about the last position considered.
   * Future calls to streamChangesSince should query from that JournalPosition
   * to avoid losing information.
   */
  ChangesSinceResult, stream<
    ChangedFileResult throws (1: eden.EdenError ex)
  > streamChangesSince(1: StreamChangesSinceParams params) throws (
    1: eden.EdenError ex,
  );

  /**
   * Returns the basic status from EdenFS as one would get from getDaemonInfo
   * and a stream of updates of the EdenFS startup process if EdenFS is
   * starting. This is the same data that would be printed to stdout during an
   * `eden start` call. The stream will be terminated when EdenFS has started.
   *
   * I wouldn't recommend introspecting each of the updates, they are really
   * just meant to be printed to a terminal for users to look at.
   */
  eden.DaemonInfo, stream<EdenStartStatusUpdate> streamStartStatus() throws (
    1: eden.EdenError ex,
  );
}
