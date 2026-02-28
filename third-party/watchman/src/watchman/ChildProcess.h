/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/futures/Future.h>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>
#include "watchman/fs/Pipe.h"
#include "watchman/portability/PosixSpawn.h"
#include "watchman/thirdparty/jansson/jansson.h"
#include "watchman/watchman_string.h"
#include "watchman/watchman_system.h"

namespace watchman {

class ChildProcess {
 public:
  struct Deleter {
    void operator()(char** vec) const {
      free((void*)vec);
    }
  };

  class Environment {
   public:
    // Constructs an environment from the current process environment
    Environment();
    Environment(const Environment&) = default;
    /* implicit */ Environment(
        const std::unordered_map<w_string, w_string>& map);

    Environment& operator=(const Environment&) = default;

    // Returns the environment as an environ compatible array
    std::unique_ptr<char*, Deleter> asEnviron(size_t* env_size = nullptr) const;

    // Set a value in the environment
    void set(const w_string& key, const w_string& value);
    void set(
        std::initializer_list<std::pair<w_string_piece, w_string_piece>> pairs);
    void setBool(const w_string& key, bool bval);

    // Remove a value from the environment
    void unset(const w_string& key);

   private:
    std::unordered_map<w_string, w_string> map_;
  };

  class Options {
   public:
    Options();
    // Not copyable
    Options(const Options&) = delete;
    Options(Options&&) = default;
    Options& operator=(const Options&) = delete;
    Options& operator=(Options&&) = default;

#ifdef POSIX_SPAWN_SETSIGMASK
    void setSigMask(const sigset_t& mask);
#endif
    // Adds flags to the set of flags maintainted in the spawn attributes.
    // This is logically equivalent to calling setflags(getflags()|flags)
    void setFlags(short flags);

    Environment& environment();

    // Arranges to duplicate an fd from the parent as targetFd in
    // the child process.
    void dup2(int sourceFd, int targetFd);
    void dup2(const FileDescriptor& fd, int targetFd);

    // Arranges to create a pipe for communicating between the
    // parent and child process and setting it as targetFd in
    // the child.
    void pipe(int targetFd, bool childRead);

    // Set up stdin with a pipe
    void pipeStdin();

    // Set up stdout with a pipe
    void pipeStdout();

    // Set up stderr with a pipe
    void pipeStderr();

    // Set up stdin with a null device
    void nullStdin();

    // Set up stdout with a null device
    void nullStdout();

    // Set up stderr with a null device
    void nullStderr();

    // Arrange to open(2) a file for the child process and make
    // it available as targetFd
    void open(int targetFd, const char* path, int flags, int mode);

    // Arrange to set the cwd for the child process
    void chdir(w_string_piece path);

   private:
    void nullFd(int fd, int flags);

    struct Inner {
      // There is no defined way to copy or move either of
      // these things, so we separate them out into a container
      // that we can point to and move the pointer.
      posix_spawn_file_actions_t actions;
      posix_spawnattr_t attr;

      Inner();
      ~Inner();
    };
    std::unique_ptr<Inner> inner_;
    Environment env_;
    std::unordered_map<int, std::unique_ptr<Pipe>> pipes_;
    std::string cwd_;

    friend class ChildProcess;
  };

  ChildProcess(std::vector<std::string_view> args, Options&& options);
  ChildProcess(const json_ref& args, Options&& options);
  ~ChildProcess();

  // Check to see if the process has terminated.
  // Does not block.  Returns true if the process has
  // terminated, false otherwise.
  bool terminated();

  // Wait for the process to terminate and return its
  // exit status.  If the process has already terminated,
  // immediately returns its exit status.
  int wait();

  // Disassociate from the running process.
  // We will no longer be able to wait for it to complete.
  // This causes minor leakage of resources.
  void disown();

  // This mutex is present to avoid fighting over the cwd when multiple
  // process might need to chdir concurrently
  static std::unique_lock<std::mutex> lockCwdMutex();

  // Terminates the process
  void kill(
#ifndef _WIN32
      int signo = SIGTERM
#endif
  );

  // If stdin is a pipe and stdout and stderr aren't, then it's safe to extract
  // the stdin pipe, write to it, close it, and then wait for the process to
  // terminate.
  std::unique_ptr<Pipe> takeStdin();

  // The pipeWriteCallback is called by communicate when it is safe to write
  // data to the pipe.  The callback should then attempt to write to it.
  // The callback must return true when it has nothing more
  // to write to the input of the child.  This will cause the
  // pipe to be closed.
  // Note that the pipe may be non-blocking, and you must not loop attempting
  // to write data to the pipe - the caller will arrange to call you again
  // if you return false (e.g. after a partial write).
  using pipeWriteCallback = std::function<bool(FileDescriptor&)>;

  /** ChildProcess::communicate() performs a read/write operation.
   * The provided pipeWriteCallback allows sending data to the input stream.
   * communicate() will return with the pair of output and error streams once
   * they have been completely consumed. */
  std::pair<std::optional<w_string>, std::optional<w_string>> communicate(
      pipeWriteCallback writeCallback = [](FileDescriptor&) {
        // If not provided by the caller, we're just going to close the input
        // stream
        return true;
      });

  // these are public for the sake of testing.  You should use the
  // communicate() method instead of calling these directly.
  std::pair<std::optional<w_string>, std::optional<w_string>>
  pollingCommunicate(pipeWriteCallback writable);
  std::pair<std::optional<w_string>, std::optional<w_string>>
  threadedCommunicate(pipeWriteCallback writable);

  /**
   * Return the maximum number of platform characters allowed in the command
   * lines, including null terminators. On POSIX, this number also includes the
   * space consumed by environment variables.
   */
  static size_t getArgMax();

 private:
  pid_t pid_;
  bool waited_{false};
  int status_;
  std::unordered_map<int, std::unique_ptr<Pipe>> pipes_;

  folly::Future<std::optional<w_string>> readPipe(int fd);
};
} // namespace watchman
