/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "watchman/scm/Git.h"
#include <fmt/core.h>
#include <folly/String.h>
#include <folly/portability/SysTime.h>
#include "watchman/ChildProcess.h"
#include "watchman/CommandRegistry.h"
#include "watchman/Logging.h"
#include "watchman/fs/FileSystem.h"

// Capability indicating support for the git SCM
W_CAP_REG("scm-git")

using namespace std::chrono;

namespace {
using namespace watchman;

void replaceEmbeddedNulls(std::string& str) {
  std::replace(str.begin(), str.end(), '\0', '\n');
}

std::string gitExecutablePath() {
  return "git";
}

struct GitResult {
  w_string output;
};

GitResult runGit(
    std::vector<std::string_view> cmdline,
    ChildProcess::Options options,
    std::string_view description) {
  ChildProcess proc{cmdline, std::move(options)};
  auto outputs = proc.communicate();
  auto status = proc.wait();
  if (status) {
    auto output =
        std::string{outputs.first ? outputs.first->view() : std::string_view{}};
    auto error = std::string{
        outputs.second ? outputs.second->view() : std::string_view{}};
    replaceEmbeddedNulls(output);
    replaceEmbeddedNulls(error);

    SCMError::throwf(
        "failed to {}\ncmd = {}\nstdout = {}\nstderr = {}\nstatus = {}",
        description,
        folly::join(" ", cmdline),
        output,
        error,
        status);
  }

  if (outputs.first) {
    return GitResult{std::move(*outputs.first)};
  } else {
    return GitResult{""};
  }
}

} // namespace

namespace watchman {

Git::Git(w_string_piece rootPath, w_string_piece scmRoot)
    : SCM(rootPath, scmRoot),
      indexPath_(fmt::format("{}/.git/index", getSCMRoot())),
      commitsPrior_(Configuration(), "scm_git_commits_prior", 32, 10),
      mergeBases_(Configuration(), "scm_git_mergebase", 32, 10),
      filesChangedSinceMergeBaseWith_(
          Configuration(),
          "scm_git_files_since_mergebase",
          32,
          10) {}

ChildProcess::Options Git::makeGitOptions(
    const std::optional<w_string>& requestId) const {
  ChildProcess::Options opt;
  (void)requestId;
  opt.nullStdin();
  opt.pipeStdout();
  opt.pipeStderr();
  opt.chdir(getRootPath());
  return opt;
}

struct timespec Git::getIndexMtime() const {
  try {
    auto info =
        getFileInformation(indexPath_.c_str(), CaseSensitivity::CaseSensitive);
    return info.mtime;
  } catch (const std::system_error&) {
    // Failed to stat, so assume the current time
    struct timeval now;
    gettimeofday(&now, nullptr);
    struct timespec ts;
    ts.tv_sec = now.tv_sec;
    ts.tv_nsec = now.tv_usec * 1000;
    return ts;
  }
}

w_string Git::mergeBaseWith(
    w_string_piece commitId,
    const std::optional<w_string>& requestId) const {
  auto mtime = getIndexMtime();
  auto key = fmt::format("{}:{}:{}", commitId, mtime.tv_sec, mtime.tv_nsec);
  auto commit = std::string{commitId.view()};

  return mergeBases_
      .get(
          key,
          [this, commit, requestId](const std::string&) {
            auto result = runGit(
                {gitExecutablePath(), "merge-base", commit, "HEAD"},
                makeGitOptions(requestId),
                "query for the merge base");

            auto output = std::string{result.output.view()};
            if (!output.empty() && output.back() == '\n') {
              output.pop_back();
            }

            if (output.size() != 40) {
              SCMError::throwf(
                  "expected merge base to be a 40 character string, got {}",
                  output);
            }

            // TODO: is w_string(s.c_str()) safe?
            return folly::makeFuture(w_string(output.c_str()));
          })
      .get()
      ->value();
}

std::vector<w_string> Git::getFilesChangedSinceMergeBaseWith(
    w_string_piece commitId,
    w_string_piece clock,
    const std::optional<w_string>& requestId) const {
  auto key = fmt::format("{}:{}", commitId, clock);
  auto commitCopy = std::string{commitId.view()};
  return filesChangedSinceMergeBaseWith_
      .get(
          key,
          [this, commit = std::move(commitCopy), requestId](
              const std::string&) {
            auto result = runGit(
                {gitExecutablePath(), "diff", "--name-only", "-z", commit},
                makeGitOptions(requestId),
                "query for files changed since merge base");

            std::vector<w_string> lines;
            w_string_piece(result.output).split(lines, '\0');
            return folly::makeFuture(lines);
          })
      .get()
      ->value();
}

std::chrono::time_point<std::chrono::system_clock> Git::getCommitDate(
    w_string_piece commitId,
    const std::optional<w_string>& requestId) const {
  auto result = runGit(
      {gitExecutablePath(), "log", "--format:%ct", "-n", "1", commitId.view()},
      makeGitOptions(requestId),
      "get commit date");
  double timestamp;
  if (std::sscanf(result.output.c_str(), "%lf", &timestamp) != 1) {
    throw std::runtime_error(fmt::format(
        "failed to parse date value `{}` into a double", result.output));
  }
  return system_clock::from_time_t(timestamp);
}

std::vector<w_string> Git::getCommitsPriorToAndIncluding(
    w_string_piece commitId,
    int numCommits,
    const std::optional<w_string>& requestId) const {
  auto mtime = getIndexMtime();
  auto key = fmt::format(
      "{}:{}:{}:{}", commitId, numCommits, mtime.tv_sec, mtime.tv_nsec);
  auto commitCopy = std::string{commitId.view()};

  return commitsPrior_
      .get(
          key,
          [this, commit = std::move(commitCopy), numCommits, requestId](
              const std::string&) {
            auto result = runGit(
                {gitExecutablePath(),
                 "log",
                 "-n",
                 fmt::to_string(numCommits),
                 "--format=%H",
                 commit},
                makeGitOptions(requestId),
                "get prior commits");

            std::vector<w_string> lines;
            w_string_piece(result.output).split(lines, '\n');
            return folly::makeFuture(lines);
          })
      .get()
      ->value();
}

} // namespace watchman
