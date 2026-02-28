/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "watchman/Errors.h"

#ifdef _WIN32
#include <winerror.h> // @manual
#endif

using std::generic_category;

namespace watchman {

const char* error_category::name() const noexcept {
  return "watchman";
}

std::string error_category::message(int) const {
  return "the programmer should not be trying to render an error message "
         "using watchman::error_category, please report this bug!";
}

const std::error_category& error_category() {
  static class error_category cat;
  return cat;
}

const char* inotify_category::name() const noexcept {
  return "inotify";
}

std::string inotify_category::message(int err) const {
  switch (err) {
    case EMFILE:
      return "The user limit on the total number of inotify "
             "instances has been reached; increase the "
             "fs.inotify.max_user_instances sysctl";
    case ENFILE:
      return "The system limit on the total number of file descriptors "
             "has been reached";
    case ENOMEM:
      return "Insufficient kernel memory is available";
    case ENOSPC:
      return "The user limit on the total number of inotify watches "
             "was reached; increase the fs.inotify.max_user_watches sysctl";
    default:
      return std::generic_category().message(err);
  }
}

const std::error_category& inotify_category() {
  static class inotify_category cat;
  return cat;
}

bool error_category::equivalent(const std::error_code& code, int condition)
    const noexcept {
  if (code.category() == inotify_category()) {
    // Treat inotify the same as the generic category for the purposes of
    // equivalence; it is the same namespace, we just provide different
    // renditions of the error messages.
    return equivalent(
        std::error_code(code.value(), std::generic_category()), condition);
  }

  switch (static_cast<error_code>(condition)) {
    case error_code::no_such_file_or_directory:
      return
#ifdef _WIN32
          code == windows_error_code(ERROR_FILE_NOT_FOUND) ||
          code == windows_error_code(ERROR_DEV_NOT_EXIST) ||
#endif
          code == make_error_code(std::errc::no_such_file_or_directory);

    case error_code::not_a_directory:
      return
#ifdef _WIN32
          code == windows_error_code(ERROR_PATH_NOT_FOUND) ||
          code == windows_error_code(ERROR_DIRECTORY) ||
#endif
          code == make_error_code(std::errc::not_a_directory);

    case error_code::is_a_directory:
      return code == make_error_code(std::errc::is_a_directory);

#ifdef ESTALE
    case error_code::stale_file_handle:
      return code == std::error_code(ESTALE, std::generic_category());
#endif

    case error_code::too_many_symbolic_link_levels:
      // POSIX says open with O_NOFOLLOW should set errno to ELOOP if the path
      // is a symlink. However, FreeBSD (which ironically originated O_NOFOLLOW)
      // sets it to EMLINK.  So we check for either condition here.
      return code ==
          make_error_code(std::errc::too_many_symbolic_link_levels) ||
          code == make_error_code(std::errc::too_many_links);

    case error_code::permission_denied:
      return
#ifdef _WIN32
          code == windows_error_code(ERROR_ACCESS_DENIED) ||
          code == windows_error_code(ERROR_INVALID_ACCESS) ||
          code == windows_error_code(ERROR_WRITE_PROTECT) ||
          code == windows_error_code(ERROR_SHARING_VIOLATION) ||
#endif
          code == make_error_code(std::errc::permission_denied) ||
          code == make_error_code(std::errc::operation_not_permitted);

    case error_code::system_limits_exceeded:
      return
#ifdef _WIN32
          code == windows_error_code(ERROR_TOO_MANY_OPEN_FILES) ||
#endif
          code == make_error_code(std::errc::too_many_files_open_in_system) ||
          code == make_error_code(std::errc::no_space_on_device) ||
          code == make_error_code(std::errc::not_enough_memory) ||
          code == make_error_code(std::errc::too_many_files_open);

    case error_code::timed_out:
      return
#ifdef _WIN32
          code == windows_error_code(ERROR_TIMEOUT) ||
          code == windows_error_code(WAIT_TIMEOUT) ||
#endif
          code == make_error_code(std::errc::timed_out);

    case error_code::not_a_symlink:
      return
#ifdef _WIN32
          code == windows_error_code(ERROR_NOT_A_REPARSE_POINT) ||
#endif
          code == make_error_code(std::errc::invalid_argument);

    default:
      return false;
  }
}
} // namespace watchman
