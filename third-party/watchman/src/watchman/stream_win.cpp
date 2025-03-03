/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <algorithm>
#include <memory>
#include "watchman/Logging.h"
#include "watchman/portability/WinError.h"
#include "watchman/watchman_stream.h"

using namespace watchman;

#ifdef _WIN32

#include "eden/common/utils/WinUtil.h"

// Things are more complicated here than on unix.
// We maintain an overlapped context for reads and
// another for writes.  Actual write data is queued
// and dispatched to the underlying handle as prior
// writes complete.

class win_handle;

namespace {
class WindowsEvent : public watchman_event {
 public:
  HANDLE hEvent;

  explicit WindowsEvent(bool initialState = false)
      : hEvent(CreateEvent(nullptr, TRUE, initialState, nullptr)) {}

  ~WindowsEvent() {
    CloseHandle(hEvent);
  }

  void notify() override {
    SetEvent(hEvent);
  }

  bool testAndClear() override {
    bool was_set = WaitForSingleObject(hEvent, 0) == WAIT_OBJECT_0;
    ResetEvent(hEvent);
    return was_set;
  }

  void reset() {
    ResetEvent(hEvent);
  }

  FileDescriptor::system_handle_type system_handle() override {
    return (FileDescriptor::system_handle_type)hEvent;
  }

  bool isSocket() override {
    return false;
  }
};
} // namespace

struct overlapped_op {
  OVERLAPPED olap;
  class win_handle* h;
  struct write_buf* wbuf;
};

struct write_buf {
  struct write_buf* next;
  int len;
  char* cursor;
  char data[1];
};

class win_handle : public watchman_stream {
 public:
  struct overlapped_op *read_pending{nullptr}, *write_pending{nullptr};
  FileDescriptor h;
  WindowsEvent waitable;
  CRITICAL_SECTION mtx;
  bool error_pending{false};
  DWORD errcode{0};
  DWORD file_type;
  struct write_buf *write_head{nullptr}, *write_tail{nullptr};
  char read_buf[8192];
  char* read_cursor{read_buf};
  int read_avail{0};
  bool blocking{true};

  explicit win_handle(FileDescriptor&& handle);
  ~win_handle();
  int read(void* buf, int size) override;
  int write(const void* buf, int size) override;
  watchman_event* getEvents() override;
  void setNonBlock(bool nonb) override;
  bool rewind() override;
  bool shutdown() override;
  bool peerIsOwner() override;
  const FileDescriptor& getFileDescriptor() const override {
    return h;
  }

  // Helper to avoid sprinkling casts all over this file
  inline HANDLE handle() const {
    return (HANDLE)h.handle();
  }

  pid_t getPeerProcessID() const override {
    return facebook::eden::getPeerProcessID(h.system_handle());
  }
};

#if 1
#define stream_debug(x, ...) 0
#else
#define stream_debug(x, ...)                                      \
  do {                                                            \
    time_t now;                                                   \
    char timebuf[64];                                             \
    struct tm tm;                                                 \
    time(&now);                                                   \
    localtime_s(&tm, &now);                                       \
    strftime(timebuf, sizeof(timebuf), "%Y-%m-%dT%H:%M:%S", &tm); \
    fprintf(stderr, "%s    : ", timebuf);                         \
    fprintf(stderr, x, __VA_ARGS__);                              \
    fflush(stderr);                                               \
  } while (0)
#endif

typedef BOOL(WINAPI* get_overlapped_result_ex_func)(
    HANDLE file,
    LPOVERLAPPED olap,
    LPDWORD bytes,
    DWORD millis,
    BOOL alertable);
static BOOL WINAPI probe_get_overlapped_result_ex(
    HANDLE file,
    LPOVERLAPPED olap,
    LPDWORD bytes,
    DWORD millis,
    BOOL alertable);
static get_overlapped_result_ex_func get_overlapped_result_ex =
    probe_get_overlapped_result_ex;

static BOOL WINAPI get_overlapped_result_ex_impl(
    HANDLE file,
    LPOVERLAPPED olap,
    LPDWORD bytes,
    DWORD millis,
    BOOL alertable) {
  DWORD waitReturnCode, err;

  stream_debug("Preparing to wait for maximum %ums\n", millis);
  if (millis != 0) {
    waitReturnCode = WaitForSingleObjectEx(olap->hEvent, millis, alertable);
    switch (waitReturnCode) {
      case WAIT_OBJECT_0:
        // Event is signaled, overlapped IO operation result should be
        // available.
        break;
      case WAIT_IO_COMPLETION:
        // WaitForSingleObjectEx returnes because the system added an I/O
        // completion routine or an asynchronous procedure call (APC) to the
        // thread queue.
        SetLastError(WAIT_IO_COMPLETION);
        break;
      case WAIT_TIMEOUT:
        // We reached the maximum allowed wait time, the IO operation failed
        // to complete in timely fashion.
        SetLastError(WAIT_TIMEOUT);
        return FALSE;

      case WAIT_FAILED:
        // something went wrong calling WaitForSingleObjectEx
        err = GetLastError();
        stream_debug("WaitForSingleObjectEx failed: %s\n", win32_strerror(err));
        return FALSE;

      default:
        // unexpected situation deserving investigation.
        err = GetLastError();
        stream_debug("Unexpected error: %s\n", win32_strerror(err));
        return FALSE;
    }
  }

  return GetOverlappedResult(file, olap, bytes, FALSE);
}

static BOOL WINAPI probe_get_overlapped_result_ex(
    HANDLE file,
    LPOVERLAPPED olap,
    LPDWORD bytes,
    DWORD millis,
    BOOL alertable) {
  get_overlapped_result_ex_func func;

  func = (get_overlapped_result_ex_func)GetProcAddress(
      GetModuleHandle("kernel32.dll"), "GetOverlappedResultEx");

  if ((getenv("WATCHMAN_WIN7_COMPAT") &&
       getenv("WATCHMAN_WIN7_COMPAT")[0] == '1') ||
      !func) {
    func = get_overlapped_result_ex_impl;
  }

  get_overlapped_result_ex = func;

  return func(file, olap, bytes, millis, alertable);
}

win_handle::~win_handle() {
  EnterCriticalSection(&mtx);

  if (read_pending) {
    if (CancelIoEx(handle(), &read_pending->olap)) {
      free(read_pending);
      read_pending = nullptr;
    }
  }
  if (write_pending) {
    if (CancelIoEx(handle(), &write_pending->olap)) {
      free(write_pending);
      write_pending = nullptr;
    }

    while (write_head) {
      struct write_buf* b = write_head;
      write_head = b->next;

      free(b);
    }
  }

  DeleteCriticalSection(&mtx);
}

static void move_from_read_buffer(
    class win_handle* h,
    int* total_read_ptr,
    char** target_buf_ptr,
    int* size_ptr) {
  int nread = std::min(*size_ptr, h->read_avail);
  size_t wasted;

  if (!nread) {
    return;
  }

  memcpy(*target_buf_ptr, h->read_cursor, nread);
  *total_read_ptr += nread;
  *target_buf_ptr += nread;
  *size_ptr -= nread;
  h->read_cursor += nread;
  h->read_avail -= nread;

  stream_debug("moved %d bytes from buffer\n", nread);

  // Pack the buffer to free up space at the rear for reads
  wasted = h->read_cursor - h->read_buf;
  if (wasted) {
    memmove(h->read_buf, h->read_cursor, h->read_avail);
    h->read_cursor = h->read_buf;
  }
}

static bool win_read_handle_completion(class win_handle* h) {
  BOOL olap_res;
  DWORD bytes, err;

again:

  EnterCriticalSection(&h->mtx);
  if (!h->read_pending) {
    LeaveCriticalSection(&h->mtx);
    return false;
  }

  stream_debug("have read_pending, checking status\n");
  h->waitable.reset();

  // Don't hold the mutex while we're blocked
  LeaveCriticalSection(&h->mtx);
  olap_res = get_overlapped_result_ex(
      h->handle(),
      &h->read_pending->olap,
      &bytes,
      h->blocking ? INFINITE : 0,
      true);
  err = GetLastError();
  EnterCriticalSection(&h->mtx);

  if (olap_res) {
    stream_debug(
        "pending read completed, read %d bytes, %s\n",
        (int)bytes,
        win32_strerror(err));
    h->read_avail += bytes;
    free(h->read_pending);
    h->read_pending = nullptr;
  } else {
    if (err == WAIT_IO_COMPLETION) {
      // Some other async thing completed and our wait was interrupted.
      // This is similar to EINTR
      LeaveCriticalSection(&h->mtx);
      goto again;
    }
    stream_debug("pending read failed: %s\n", win32_strerror(err));
    if (err != ERROR_IO_INCOMPLETE) {
      // Failed
      free(h->read_pending);
      h->read_pending = nullptr;

      h->errcode = err;
      h->error_pending = true;
      stream_debug("marking read as failed\n");
      h->waitable.notify();
    }
  }
  LeaveCriticalSection(&h->mtx);

  return h->read_pending != nullptr;
}

static int win_read_blocking(class win_handle* h, void* buf, int size) {
  int total_read = 0;
  DWORD bytes, err;

  move_from_read_buffer(h, &total_read, (char**)&buf, &size);

  if (size == 0) {
    return total_read;
  }

  stream_debug("blocking read of %d bytes\n", (int)size);
  if (ReadFile(h->handle(), buf, size, &bytes, nullptr)) {
    total_read += bytes;
    stream_debug(
        "blocking read provided %d bytes, total=%d\n", (int)bytes, total_read);
    return total_read;
  }

  err = GetLastError();

  stream_debug("blocking read failed: %s\n", win32_strerror(err));

  if (total_read) {
    stream_debug("but already got %d bytes from buffer\n", total_read);
    return total_read;
  }

  errno = map_win32_err(err);
  return -1;
}

static int win_read_non_blocking(class win_handle* h, void* buf, int size) {
  int total_read = 0;
  char* target;
  DWORD target_space;
  DWORD bytes;

  stream_debug("non_blocking read for %d bytes\n", size);

  move_from_read_buffer(h, &total_read, (char**)&buf, &size);

  target = h->read_cursor + h->read_avail;
  target_space = (DWORD)((h->read_buf + sizeof(h->read_buf)) - target);

  stream_debug("initiate read for %d\n", target_space);

  // Create a unique olap for each request
  h->read_pending = (overlapped_op*)calloc(1, sizeof(*h->read_pending));
  if (h->read_avail == 0) {
    stream_debug("ResetEvent because there is no read_avail right now\n");
    h->waitable.reset();
  }
  h->read_pending->olap.hEvent = h->waitable.hEvent;
  h->read_pending->h = h;

  if (!ReadFile(
          h->handle(), target, target_space, nullptr, &h->read_pending->olap)) {
    DWORD err = GetLastError();

    if (err != ERROR_IO_PENDING) {
      free(h->read_pending);
      h->read_pending = nullptr;

      stream_debug("olap read failed immediately: %s\n", win32_strerror(err));
      h->waitable.notify();
    } else {
      stream_debug("olap read queued ok\n");
    }

    errno = map_win32_err(err);
    return total_read == 0 ? -1 : total_read;
  }

  // Note: we obtain the bytes via GetOverlappedResult because the docs for
  // ReadFile warn against passing the pointer to the ReadFile parameter for
  // asynchronouse reads
  GetOverlappedResult(h->handle(), &h->read_pending->olap, &bytes, FALSE);
  stream_debug("olap read succeeded immediately bytes=%d\n", (int)bytes);

  h->read_avail += bytes;
  free(h->read_pending);
  h->read_pending = nullptr;

  move_from_read_buffer(h, &total_read, (char**)&buf, &size);

  stream_debug("read returning %d\n", total_read);
  h->waitable.notify();
  return total_read;
}

int win_handle::read(void* buf, int size) {
  if (win_read_handle_completion(this)) {
    errno = EAGAIN;
    return -1;
  }

  // Report a prior failure
  if (error_pending) {
    stream_debug(
        "win_read: reporting prior failure err=%d errno=%d %s\n",
        errcode,
        map_win32_err(errcode),
        win32_strerror(errcode));
    errno = map_win32_err(errcode);
    error_pending = false;
    return -1;
  }

  if (blocking) {
    return win_read_blocking(this, buf, size);
  }

  return win_read_non_blocking(this, buf, size);
}

static void initiate_write(class win_handle* h);

static void CALLBACK
write_completed(DWORD err, DWORD bytes, LPOVERLAPPED olap) {
  // Reverse engineer our handle from the olap pointer
  struct overlapped_op* op = (overlapped_op*)olap;
  class win_handle* h = op->h;
  struct write_buf* wbuf = op->wbuf;

  stream_debug(
      "WriteFileEx: completion callback invoked: bytes=%d %s\n",
      (int)bytes,
      win32_strerror(err));

  EnterCriticalSection(&h->mtx);
  if (h->write_pending == op) {
    h->write_pending = nullptr;
  }

  if (err == 0) {
    wbuf->cursor += bytes;
    wbuf->len -= bytes;

    if (wbuf->len == 0) {
      // Consumed this buffer
      free(wbuf);
    } else {
      stream_debug(
          "WriteFileEx: short write: %d written, %d remain\n",
          bytes,
          wbuf->len);
      // the initiate_write call will send the remainder
      // but we need to re-insert this wbuf in the write queue
      wbuf->next = h->write_head;
      h->write_head = wbuf;
      if (!h->write_tail) {
        h->write_tail = wbuf;
      }
    }
  } else {
    stream_debug("WriteFilex: completion: failed: %s\n", win32_strerror(err));
    h->errcode = err;
    h->error_pending = true;
  }

  stream_debug("SetEvent because WriteFileEx completed\n");
  h->waitable.notify();

  // Send whatever else we have waiting to go
  initiate_write(h);

  LeaveCriticalSection(&h->mtx);

  // Free the prior struct after possibly initiating another write
  // to minimize the chance of the same address being reused and
  // confusing the completion status
  free(op);
}

// Must be called with the mutex held
static void initiate_write(class win_handle* h) {
  struct write_buf* wbuf = h->write_head;
  if (h->write_pending || !wbuf) {
    return;
  }

  h->write_head = wbuf->next;
  if (!h->write_head) {
    h->write_tail = nullptr;
  }

  h->write_pending = (overlapped_op*)calloc(1, sizeof(*h->write_pending));
  h->write_pending->h = h;
  h->write_pending->wbuf = wbuf;

  stream_debug(
      "Calling WriteFileEx with wbuf=%p wbuf->cursor=%p len=%d olap=%p\n",
      wbuf,
      wbuf->cursor,
      wbuf->len,
      &h->write_pending->olap);
  if (!WriteFileEx(
          h->handle(),
          wbuf->cursor,
          wbuf->len,
          &h->write_pending->olap,
          write_completed)) {
    stream_debug("WriteFileEx: failed %s\n", win32_strerror(GetLastError()));
    free(h->write_pending);
    h->write_pending = nullptr;
  } else {
    stream_debug("WriteFileEx: queued %d bytes for later\n", wbuf->len);
  }
}

int win_handle::write(const void* buf, int size) {
  struct write_buf* wbuf;

  EnterCriticalSection(&mtx);
  if (file_type != FILE_TYPE_PIPE && blocking && !write_head) {
    DWORD bytes;
    stream_debug("blocking write of %d\n", size);
    if (WriteFile(handle(), buf, size, &bytes, nullptr)) {
      LeaveCriticalSection(&mtx);
      stream_debug("blocking write wrote %d bytes of %d\n", bytes, size);
      return bytes;
    }
    errcode = GetLastError();
    error_pending = true;
    errno = map_win32_err(errcode);
    stream_debug("SetEvent because blocking write completed (failed)\n");
    waitable.notify();
    stream_debug("write failed: %s\n", win32_strerror(errcode));
    LeaveCriticalSection(&mtx);
    return -1;
  }

  wbuf = (write_buf*)malloc(sizeof(*wbuf) + size - 1);
  if (!wbuf) {
    return -1;
  }
  wbuf->next = nullptr;
  wbuf->cursor = wbuf->data;
  wbuf->len = size;
  memcpy(wbuf->data, buf, size);

  if (write_tail) {
    write_tail->next = wbuf;
  } else {
    write_head = wbuf;
  }
  write_tail = wbuf;

  stream_debug("queue write of %d bytes to write_tail\n", size);

  if (!write_pending) {
    initiate_write(this);
  }

  LeaveCriticalSection(&mtx);

  return size;
}

watchman_event* win_handle::getEvents() {
  return &waitable;
}

void win_handle::setNonBlock(bool nonb) {
  blocking = !nonb;
}

bool win_handle::rewind() {
  bool res;
  LARGE_INTEGER new_pos;

  new_pos.QuadPart = 0;
  res = SetFilePointerEx(handle(), new_pos, &new_pos, FILE_BEGIN);
  errno = map_win32_err(GetLastError());
  return res;
}

// Ensure that any data buffered for write are sent prior to setting
// ourselves up to close
bool win_handle::shutdown() {
  BOOL olap_res;
  DWORD bytes;

  blocking = true;
  while (write_pending) {
    olap_res = get_overlapped_result_ex(
        handle(), &write_pending->olap, &bytes, INFINITE, true);
  }

  return true;
}

bool win_handle::peerIsOwner() {
  // TODO: implement this for Windows
  return true;
}

std::unique_ptr<watchman_event> w_event_make_named_pipe() {
  return std::make_unique<WindowsEvent>();
}

win_handle::win_handle(FileDescriptor&& handle)
    : h(std::move(handle)),
      // Initially signalled, meaning that they can try reading
      waitable(true),
      file_type(GetFileType((HANDLE)h.handle())) {
  InitializeCriticalSection(&mtx);
}

std::unique_ptr<watchman_stream> w_stm_fdopen_windows(FileDescriptor&& handle) {
  if (!handle) {
    return nullptr;
  }

  return std::make_unique<win_handle>(std::move(handle));
}

std::unique_ptr<watchman_stream> w_stm_connect_named_pipe(
    const char* path,
    int timeoutms) {
  DWORD err;
  DWORD64 deadline = GetTickCount64() + timeoutms;

  if (strlen(path) > 255) {
    logf(ERR, "w_stm_connect_named_pipe({}) path is too long\n", path);
    errno = E2BIG;
    return nullptr;
  }

  while (true) {
    FileDescriptor handle(
        intptr_t(CreateFile(
            path,
            GENERIC_READ | GENERIC_WRITE,
            0,
            nullptr,
            OPEN_EXISTING,
            FILE_FLAG_OVERLAPPED,
            nullptr)),
        FileDescriptor::FDType::Pipe);

    if (handle) {
      return w_stm_fdopen(std::move(handle));
    }

    err = GetLastError();
    if (timeoutms > 0) {
      timeoutms -= (DWORD)(GetTickCount64() - deadline);
    }
    if (timeoutms <= 0 ||
        (err != ERROR_PIPE_BUSY && err != ERROR_FILE_NOT_FOUND)) {
      // either we're out of time, or retrying won't help with this error
      errno = map_win32_err(err);
      return nullptr;
    }

    // We can retry
    if (!WaitNamedPipe(path, timeoutms)) {
      err = GetLastError();
      if (err == ERROR_SEM_TIMEOUT) {
        errno = map_win32_err(err);
        return nullptr;
      }
      if (err == ERROR_FILE_NOT_FOUND) {
        // Grace to allow it to be created
        SleepEx(10, true);
      }
    }
  }
}

int w_poll_events_named_pipe(EventPoll* p, int n, int timeoutms) {
  HANDLE handles[MAXIMUM_WAIT_OBJECTS];
  int i;
  DWORD res;

  if (n > MAXIMUM_WAIT_OBJECTS - 1) {
    // Programmer error :-/
    logf(
        FATAL,
        "{} > MAXIMUM_WAIT_OBJECTS-1 ({})\n",
        n,
        MAXIMUM_WAIT_OBJECTS - 1);
  }

  for (i = 0; i < n; i++) {
    auto evt = dynamic_cast<WindowsEvent*>(p[i].evt);
    w_check(evt != nullptr, "!WindowsEvent");
    handles[i] = evt->hEvent;
    p[i].ready = false;
  }

  res = WaitForMultipleObjectsEx(
      n, handles, false, timeoutms == -1 ? INFINITE : timeoutms, true);

  if (res == WAIT_FAILED) {
    errno = map_win32_err(GetLastError());
    return -1;
  }
  if (res == WAIT_IO_COMPLETION) {
    errno = EINTR;
    return -1;
  }
  // Note: WAIT_OBJECT_0 == 0
  if (/* res >= WAIT_OBJECT_0 && */ res < WAIT_OBJECT_0 + n) {
    p[res - WAIT_OBJECT_0].ready = true;
    return 1;
  }
  if (res >= WAIT_ABANDONED_0 && res < WAIT_ABANDONED_0 + n) {
    p[res - WAIT_ABANDONED_0].ready = true;
    return 1;
  }
  return 0;
}

// similar to open(2), but returns a handle
FileDescriptor w_handle_open(const char* path, int flags) {
  DWORD access = 0, share = 0, create = 0, attrs = 0;
  DWORD err;
  auto sec = SECURITY_ATTRIBUTES();

  if (!strcmp(path, "/dev/null")) {
    path = "NUL:";
  }

  auto wpath = w_string_piece(path).asWideUNC();

  if (flags & (O_WRONLY | O_RDWR)) {
    access |= GENERIC_WRITE;
  }
  if ((flags & O_WRONLY) == 0) {
    access |= GENERIC_READ;
  }

  // We want more posix-y behavior by default
  share = FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE;

  sec.nLength = sizeof(sec);
  sec.bInheritHandle = TRUE;
  if (flags & O_CLOEXEC) {
    sec.bInheritHandle = FALSE;
  }

  if ((flags & (O_CREAT | O_EXCL)) == (O_CREAT | O_EXCL)) {
    create = CREATE_NEW;
  } else if ((flags & (O_CREAT | O_TRUNC)) == (O_CREAT | O_TRUNC)) {
    create = CREATE_ALWAYS;
  } else if (flags & O_CREAT) {
    create = OPEN_ALWAYS;
  } else if (flags & O_TRUNC) {
    create = TRUNCATE_EXISTING;
  } else {
    create = OPEN_EXISTING;
  }

  attrs = FILE_ATTRIBUTE_NORMAL;
  if (flags & O_DIRECTORY) {
    attrs |= FILE_FLAG_BACKUP_SEMANTICS;
  }

  FileDescriptor h(
      intptr_t(CreateFileW(
          wpath.c_str(), access, share, &sec, create, attrs, nullptr)),
      FileDescriptor::FDType::Unknown);
  err = GetLastError();

  errno = map_win32_err(err);
  return h;
}

std::unique_ptr<watchman_stream> w_stm_open(const char* path, int flags, ...) {
  return w_stm_fdopen(w_handle_open(path, flags));
}

#endif
