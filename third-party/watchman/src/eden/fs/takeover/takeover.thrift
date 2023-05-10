/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This software may be used and distributed according to the terms of the
 * GNU General Public License version 2.
 */

namespace cpp2 facebook.eden

include "thrift/annotation/cpp.thrift"

@cpp.Type{name = "uint64_t"}
typedef i64 ui64

// A list of takeover data serialization versions that the client supports
struct TakeoverVersionQuery {
  // The set of versions supported by the client. This is on the way out.
  1: set<i32> versions;
  // The set of capabilities supported by the client. This will be the longer
  // term scheme that we use.
  2: ui64 capabilities;
}

struct SerializedInodeMapEntry {
  1: i64 inodeNumber;
  2: i64 parentInode;
  3: string name;
  4: bool isUnlinked;
  5: i64 numFsReferences;
  // If unset, the inode is materialized.
  // LEGACY: if the empty string, the inode is materialized. In the future,
  // the semantics will change here, as a BackingStore might use the empty
  // string as a valid ID.
  6: optional string hash;
  7: i32 mode;
}

struct SerializedInodeMap {
  2: list<SerializedInodeMapEntry> unloadedInodes;
}

struct SerializedFileHandleMap {}

// Mount protocol to use for a mount that we are taking over.
enum TakeoverMountProtocol {
  UNKNOWN = 0,
  FUSE = 1,
  NFS = 2,
}

struct SerializedMountInfo {
  1: string mountPath;
  2: string stateDirectory;
  // TODO: remove this field, it is no longer used
  3: list<string> bindMountPaths;
  // This binary blob is really a fuse_init_out instance.
  // We don't transcribe that into thrift because the layout
  // is system dependent and happens to be flat and thus is
  // suitable for reinterpret_cast to be used upon it to
  // access the struct once we've moved it across the process
  // boundary.  Note that takeover is always local to the same
  // machine and thus has the same endianness.
  // This will be left empty for NFS mounts.
  4: binary connInfo; // fuse_init_out

  // Removed, do not use 5
  // 5: SerializedFileHandleMap fileHandleMap,

  6: SerializedInodeMap inodeMap;

  7: TakeoverMountProtocol mountProtocol = TakeoverMountProtocol.UNKNOWN;
}

// TODO(T110300475): remove after SerializedTakeoverResult becomes stable. Should be
// removable in March 2022.
// Deprecated only old versions of EdenFS will send takeover data using this
// struct. All new versions should use SerializedTakeoverResult. This is because
// SerializedTakeoverData does not allow us to send non mount specific data in
// the non error case.
union SerializedTakeoverData {
  1: list<SerializedMountInfo> mounts;
  2: string errorReason;
}

enum FileDescriptorType {
  LOCK_FILE = 0,
  THRIFT_SOCKET = 1,
  MOUNTD_SOCKET = 2,
}

struct SerializedTakeoverInfo {
  1: list<SerializedMountInfo> mounts;
  2: list<FileDescriptorType> fileDescriptors;
}

// This is the highlevel structure we use to send takeover data between the
// client and server.
union SerializedTakeoverResult {
  1: SerializedTakeoverInfo takeoverData;
  2: string errorReason;
}
