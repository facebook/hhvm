/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 */

#include <caml/mlvalues.h>
#include <caml/memory.h>
#include <caml/alloc.h>
#include <caml/custom.h>
#include <caml/fail.h>
#include <caml/signals.h>
#include <caml/callback.h>



 // We've raided the OSX events system, as we'd have to do the same on windows.
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Commands and events are passed back and forth between the main thread and
 * the run loop thread using two lockless linked lists. Each linked list is
 * only prepended to by one thread and only cleared by the other thread. Here's
 * a quick explanation of how they work:
 *
 * So lets say the linked list of events looks like
 *
 * events = C -> B -> A
 *
 * and we have a new event D to put on this list. We point D at C and try to
 * atomically set events to point at D if and only if events still points at C.
 * If this works we have
 *
 * events = D -> C -> B -> A
 *
 * If this exchange fails, that means events was changed by the other thread.
 * The other thread can clear the list, so we know events can only be NULL
 * until we change it. So the state is now
 *
 * events = NULL.
 *
 * In theory we could just set events to D. I still use the compareAndSwap to
 * assert this invariant though. Now the state is
 *
 * events = D
 *
 * From the reading side, it looks like this: Let's say the state is
 *
 * events = C -> B -> A
 *
 * So we'll set to_do = C and try to swap NULL into events atomically if and
 * only if events still points to C. If it works, the state will be
 *
 * events = NULL
 *
 * If it fails, then the writing thread must have added something to the list.
 *
 * So the state might look like
 *
 * events = D -> C -> B -> A
 *
 * So we just try again until it works.
 */
typedef enum { ADD_WATCH, RM_WATCH } command_type;
struct command {
  struct command *next;
  char *arg;
  command_type type;
};

struct event {
  struct event *next;
  char *wpath;
  char *path;
};

// Terrible guesstimate of ~1k files, highly unlikely,
// but it's ~16kb buffer.
#define FILE_NOTIFY_BUFFER_LENGTH ((sizeof(DWORD) * 4) * 1000)
#define FILE_NOTIFY_CONDITIONS (\
FILE_NOTIFY_CHANGE_FILE_NAME | \
FILE_NOTIFY_CHANGE_DIR_NAME | \
FILE_NOTIFY_CHANGE_ATTRIBUTES | \
FILE_NOTIFY_CHANGE_SIZE | \
FILE_NOTIFY_CHANGE_LAST_WRITE | \
FILE_NOTIFY_CHANGE_SECURITY | \
FILE_NOTIFY_CHANGE_CREATION)

/**
 * This is a structure that the watcher thread holds on to. It has all the context it
 * needs
 */
struct thread_env {
  // This fd will be ready to be read when there is a command ready to be run
  HANDLE read_command_fd;
  // The thread writes to this fd when there are events ready to be returned
  HANDLE write_event_fd;
  // A pointer to the linked list of commands to run. The thread will only ever
  // remove all the commands...it will never append a command
  struct command **commands;
  // A pointer to the linked list of events to process. The thread will append
  // events to this linked list
  struct event **events;
  // The buffer used by the callbacks.
  char file_notify_buffer[FILE_NOTIFY_BUFFER_LENGTH];
  // Just in case numbers all work out and we need the extra space
  // for the null at the end of the filename.
  wchar_t bonus_buffer;
};

struct filewatch_env {
  const char* watched_directory;
  thread_env* parent_env;
  HANDLE dir_handle;
};

/**
 * This is a structure that holds all the context the main thread needs. A
 * pointer to this structure will be returned to ocaml and passed in whenever
 * something needs to be done
 */
struct env {
  // The main thread writes to this fd when there are events ready to be run
  HANDLE write_command_fd;
  // This fd will be ready to be read when there are events ready to be
  // processed
  HANDLE read_event_fd;
  // A pointer to the linked list of commands to run. The main thread will
  // append commands to this list
  struct command **commands;
  // A pointer to the linked list of events to process. The main thread will
  // only ever remove all the events...it will never append an event
  struct event **events;
};

/**
 * The pipes are only used signal the other thread that something is ready. So
 * writing '.' to the pipe is sufficient. In theory, you could get away with
 * only having pipes and not using the linked lists at all. pthreads can
 * automically write to and read from pipes up to N bytes (which I think is
 * usually 512). In the case where we have large paths, though, it gets a
 * little tricker, and I like writing lockless data structures :P
 */
static void signal_on_pipe(HANDLE fd) {
  char dot = '.';
  DWORD bytesWritten;
  WriteFile(fd, &dot, sizeof(dot), &bytesWritten, NULL);
}

/**
 * Cleans out all the data on a pipe. The other thread has signaled us on this
 * pipe and we've received the signal, now we need to read all the data so we
 * can select on this fd again.
 */
static void clear_pipe(HANDLE fd) {
  char c;
  DWORD readLen;
  while (ReadFile(fd, &c, sizeof(c), &readLen, NULL)) {
  }
}

static void CALLBACK watch_callback(DWORD errorCode, DWORD numberOfBytesTransferred, LPOVERLAPPED overlappedBuffer)
{
  filewatch_env* watchEnv = (filewatch_env*)overlappedBuffer->hEvent;

  FILE_NOTIFY_INFORMATION* fileInfo = (FILE_NOTIFY_INFORMATION*)&watchEnv->parent_env->file_notify_buffer;

  for (;;) {
    // Forcefully null terminate the filename.
    wchar_t oldMem = fileInfo->FileName[fileInfo->FileNameLength];
    fileInfo->FileName[fileInfo->FileNameLength] = L'\0';

    char* modifiedFilename = (char*)malloc(sizeof(wchar_t) * fileInfo->FileNameLength);
    size_t filenameLen = wcstombs_s(NULL, modifiedFilename, sizeof(wchar_t) * fileInfo->FileNameLength, fileInfo->FileName, (sizeof(wchar_t) * fileInfo->FileNameLength) - 1);
    fileInfo->FileName[fileInfo->FileNameLength] = oldMem;
    modifiedFilename = (char*)realloc(modifiedFilename, filenameLen +  1);

    struct event *ev = (struct event*)malloc(sizeof(struct event));
    ev->path = modifiedFilename;
    ev->wpath = _strdup(watchEnv->watched_directory);

    // Update the lockless linked list of events
    ev->next = *(watchEnv->parent_env->events);
    if (InterlockedCompareExchangePointer((void**)(watchEnv->parent_env->events), ev, ev->next) != ev) {
      // The only thing the main thread can write to events is NULL, so we can just
      // try one more time
      ev->next = NULL;
      if (InterlockedCompareExchangePointer((void**)(watchEnv->parent_env->events), ev, ev->next) != ev) {
        // This should never happen
        fprintf(stderr, "Unexpected error with fsevents lockless events list\n");
      }
    }
    signal_on_pipe(watchEnv->parent_env->write_event_fd);

    if (fileInfo->NextEntryOffset == 0)
      break;
    fileInfo = (FILE_NOTIFY_INFORMATION*)((char*)fileInfo + fileInfo->NextEntryOffset);
  }

  ReadDirectoryChangesW(
    watchEnv->dir_handle,
    &watchEnv->parent_env->file_notify_buffer,
    FILE_NOTIFY_BUFFER_LENGTH,
    true,
    FILE_NOTIFY_CONDITIONS,
    NULL,
    overlappedBuffer,
    &watch_callback);
}

/**
 * Create a new watch for a given path
 */
static void add_watch(char *wpath, struct thread_env *env) {
  // Yes, this is technically a memory leak, but we don't support removing
  // watches anyways.
  filewatch_env* watchEnv = (filewatch_env*)malloc(sizeof(filewatch_env));
  watchEnv->watched_directory = _strdup(wpath);
  watchEnv->parent_env = env;
  OVERLAPPED* overlappedBuffer = (OVERLAPPED*)calloc(1, sizeof(OVERLAPPED));
  // As backed up by the documentation, hEvent isn't used when a callback is passed.
  overlappedBuffer->hEvent = watchEnv;
  watchEnv->dir_handle = CreateFile(wpath,
    FILE_LIST_DIRECTORY, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
    NULL,
    OPEN_EXISTING,
    FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
    NULL);
  ReadDirectoryChangesW(
    watchEnv->dir_handle,
    &env->file_notify_buffer,
    FILE_NOTIFY_BUFFER_LENGTH, 
    true,
    FILE_NOTIFY_CONDITIONS,
    NULL,
    overlappedBuffer,
    &watch_callback);
}

/**
 * The run loop thread is responsible for running the loop and then executing
 * the callbacks when they are triggered
 */
static void run_loop_thread(struct thread_env *thread_env) {
  HANDLE commandReadHandle = (HANDLE)thread_env->read_command_fd;

  while (true) {
    DWORD rc = WaitForMultipleObjectsEx(1, &commandReadHandle, false, INFINITE, true);
    switch (rc) {
      case WAIT_OBJECT_0: {
        struct command *to_do = NULL;
        struct command *to_free = NULL;
        clear_pipe(thread_env->read_command_fd);
        do {
          to_do = *(thread_env->commands);
        } while (InterlockedCompareExchangePointer((void**)(thread_env->commands), NULL, to_do) != NULL);
        while (to_do != NULL) {
          switch (to_do->type) {
            case ADD_WATCH:
              add_watch(to_do->arg, thread_env);
              break;
            case RM_WATCH:
              // hh_server doesn't need this at the moment. Shouldn't be too hard to
              // do...just need to map of paths to FSEvent streams.
              fprintf(stderr, "fsevents impl doesn't support removing watches yet\n");
              break;
          }
          to_free = to_do;
          to_do = to_do->next;
          free(to_free->arg);
          free(to_free);
        }
        break;
      }
      case WAIT_IO_COMPLETION:
        // Nothing to do.
        break;
      default:
        break;
    }
  }
}

/**
 * Starts up a thread running a run loop, creates the env, and then returns it
 */
static struct env *fsevents_init()
{
  pthread_t loop_thread;
  struct thread_env *thread_env;
  struct env *env;

  struct command **commands = (struct command**)malloc(sizeof(struct command*));
  struct event **events = (struct event**)malloc(sizeof(struct event*));
  *commands = NULL;
  *events = NULL;

  thread_env = (struct thread_env*)malloc(sizeof(struct thread_env));
  thread_env->commands = commands;
  thread_env->events = events;

  env = (struct env*)malloc(sizeof(struct env));
  env->commands = commands;
  env->events = events;

  CreatePipe(&thread_env->read_command_fd, &env->write_command_fd, NULL, 0);
  CreatePipe(&env->read_event_fd, &thread_env->write_event_fd, NULL, 0);

  pthread_create(&loop_thread, NULL, (void*(*)(void*))&run_loop_thread, thread_env);

  return env;
}

/**
 * Sends a command to the run loop thread by adding the command to the linked
 * list of commands and then signaling the run loop thread through a pipe
 */
static void send_command(struct env *env, command_type type, char const *arg) {
  struct command *c = (struct command*)malloc(sizeof(struct command));

  c->type = type;
  c->arg = (char*)malloc(strlen(arg) + 1);
  strcpy(c->arg, arg);
  c->next = *(env->commands);
  if (InterlockedCompareExchangePointer((void**)(env->commands), c, c->next) != c) {
    // The only thing the thread can write to commands is NULL, so we can just
    // try one more time
    c->next = NULL;
    if (InterlockedCompareExchangePointer((void**)(env->commands), c, c->next) != c) {
      // This should never happen
      fprintf(stderr, "Unexpected error with fsevents lockless commands list\n");
    }
  }
  signal_on_pipe(env->write_command_fd);
}

/**
 * Grabs the events that are ready to be processed from the linked list and
 * clears the list
 */
static struct event *read_events(struct env *env) {
  struct event *to_do = NULL;
  clear_pipe(env->read_event_fd);
  do {
    to_do = *(env->events);
  } while (InterlockedCompareExchangePointer((void**)(env->events), NULL, to_do) != NULL);
  return to_do;
}

value stub_fsevents_init(value unit)
{
  // We're returning a pointer to ocaml land. This type will be opaque to them
  // and only be useful when passed back to c. This is a safe way to pass
  // pointers back to ocaml. See
  // http://caml.inria.fr/pub/docs/manual-ocaml/intfc.html#sec412
  return (value)fsevents_init();
}

value stub_fsevents_add_watch(value env, value path)
{
  CAMLparam2(env, path);
  CAMLlocal1(ret);
  struct env *c_env = (struct env*)env;
  char output[_MAX_PATH];
  char *rpath = _strdup(_fullpath(output, String_val(path), _MAX_PATH));
  send_command(c_env, ADD_WATCH, rpath);
  ret = caml_copy_string(rpath);
  free(rpath);
  CAMLreturn(ret);
}

/**
 * This functionality is not yet implemented
 */
value stub_fsevents_rm_watch(value env, value path)
{
  CAMLparam2(env, path);
  CAMLlocal1(ret);
  struct env *c_env = (struct env*)env;
  char output[_MAX_PATH];
  char *rpath = _strdup(_fullpath(output, String_val(path), _MAX_PATH));
  send_command(c_env, RM_WATCH, String_val(path));
  ret = caml_copy_string(rpath);
  free(rpath);
  CAMLreturn(ret);
}

value stub_fsevents_get_event_fd(value env)
{
  CAMLparam1(env);
  HANDLE fd = ((struct env *)env)->read_event_fd;
  CAMLreturn((value)fd);
}

value stub_fsevents_read_events(value env)
{
  CAMLparam1(env);
  struct event *to_free;
  struct event *events = read_events((struct env*)env);
  CAMLlocal3(event_list, event, cons);
  event_list = Val_emptylist;
  while (events != NULL) {
    // A tuple to store the filed
    event = caml_alloc(2, 0);
    Store_field(event, 0, caml_copy_string(events->path));
    Store_field(event, 1, caml_copy_string(events->wpath));
    to_free = events;
    events = events->next;

    // This is how you do event::event_list in c
    cons = caml_alloc(2, 0);
    Store_field(cons, 0, event);
    Store_field(cons, 1, event_list);
    event_list = cons;

    // Free the processed event
    free(to_free->path);
    free(to_free->wpath);
    free(to_free);
  }

  CAMLreturn(event_list);
}

#ifdef __cplusplus
}
#endif