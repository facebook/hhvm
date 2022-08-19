/* Copyright (c) 2008, 2020, Oracle and/or its affiliates. All rights reserved.

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License, version 2.0,
  as published by the Free Software Foundation.

  This program is also distributed with certain software (including
  but not limited to OpenSSL) that is licensed under separate terms,
  as designated in a particular file or component or in included license
  documentation.  The authors of MySQL hereby grant you an additional
  permission to link the program and your derivative works with the
  separately licensed software that they have included with MySQL.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License, version 2.0, for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA */

#ifndef COMPONENTS_SERVICES_PSI_FILE_BITS_H
#define COMPONENTS_SERVICES_PSI_FILE_BITS_H

#ifndef MYSQL_ABI_CHECK
#include <stddef.h> /* size_t */
#endif

#include <mysql/components/services/my_io_bits.h> /* File */

/**
  @file
  Performance schema instrumentation interface.

  @defgroup psi_abi_file File Instrumentation (ABI)
  @ingroup psi_abi
  @{
*/

/**
  Instrumented file key.
  To instrument a file, a file key must be obtained using @c register_file.
  Using a zero key always disable the instrumentation.
*/
typedef unsigned int PSI_file_key;

/**
  Interface for an instrumented file handle.
  This is an opaque structure.
*/
struct PSI_file;
typedef struct PSI_file PSI_file;

/**
  Interface for an instrumented file operation.
  This is an opaque structure.
*/
struct PSI_file_locker;
typedef struct PSI_file_locker PSI_file_locker;

/** Operation performed on an instrumented file. */
enum PSI_file_operation {
  /** File creation, as in @c create(). */
  PSI_FILE_CREATE = 0,
  /** Temporary file creation, as in @c create_temp_file(). */
  PSI_FILE_CREATE_TMP = 1,
  /** File open, as in @c open(). */
  PSI_FILE_OPEN = 2,
  /** File open, as in @c fopen(). */
  PSI_FILE_STREAM_OPEN = 3,
  /** File close, as in @c close(). */
  PSI_FILE_CLOSE = 4,
  /** File close, as in @c fclose(). */
  PSI_FILE_STREAM_CLOSE = 5,
  /**
    Generic file read, such as @c fgets(), @c fgetc(), @c fread(), @c read(),
    @c pread().
  */
  PSI_FILE_READ = 6,
  /**
    Generic file write, such as @c fputs(), @c fputc(), @c fprintf(),
    @c vfprintf(), @c fwrite(), @c write(), @c pwrite().
  */
  PSI_FILE_WRITE = 7,
  /** Generic file seek, such as @c fseek() or @c seek(). */
  PSI_FILE_SEEK = 8,
  /** Generic file tell, such as @c ftell() or @c tell(). */
  PSI_FILE_TELL = 9,
  /** File flush, as in @c fflush(). */
  PSI_FILE_FLUSH = 10,
  /** File stat, as in @c stat(). */
  PSI_FILE_STAT = 11,
  /** File stat, as in @c fstat(). */
  PSI_FILE_FSTAT = 12,
  /** File chsize, as in @c my_chsize(). */
  PSI_FILE_CHSIZE = 13,
  /** File delete, such as @c my_delete() or @c my_delete_with_symlink(). */
  PSI_FILE_DELETE = 14,
  /** File rename, such as @c my_rename() or @c my_rename_with_symlink(). */
  PSI_FILE_RENAME = 15,
  /** File sync, as in @c fsync() or @c my_sync(). */
  PSI_FILE_SYNC = 16
};
typedef enum PSI_file_operation PSI_file_operation;

/**
  File instrument information.
  @since PSI_FILE_VERSION_1
  This structure is used to register an instrumented file.
*/
struct PSI_file_info_v1 {
  /**
    Pointer to the key assigned to the registered file.
  */
  PSI_file_key *m_key;
  /**
    The name of the file instrument to register.
  */
  const char *m_name;
  /**
    The flags of the file instrument to register.
    @sa PSI_FLAG_SINGLETON
  */
  unsigned int m_flags;
  /** Volatility index. */
  int m_volatility;
  /** Documentation. */
  const char *m_documentation;
};
typedef struct PSI_file_info_v1 PSI_file_info_v1;

/**
  State data storage for @c get_thread_file_name_locker_v1_t.
  This structure provide temporary storage to a file locker.
  The content of this structure is considered opaque,
  the fields are only hints of what an implementation
  of the psi interface can use.
  This memory is provided by the instrumented code for performance reasons.
  @sa get_thread_file_name_locker_v1_t
  @sa get_thread_file_stream_locker_v1_t
  @sa get_thread_file_descriptor_locker_v1_t
*/
struct PSI_file_locker_state_v1 {
  /** Internal state. */
  unsigned int m_flags;
  /** Current operation. */
  enum PSI_file_operation m_operation;
  /** Current file. */
  struct PSI_file *m_file;
  /** Current file name. */
  const char *m_name;
  /** Current file class. */
  void *m_class;
  /** Current thread. */
  struct PSI_thread *m_thread;
  /** Operation number of bytes. */
  size_t m_number_of_bytes;
  /** Timer start. */
  unsigned long long m_timer_start;
  /** Timer function. */
  unsigned long long (*m_timer)(void);
  /** Internal data. */
  void *m_wait;
};
typedef struct PSI_file_locker_state_v1 PSI_file_locker_state_v1;

/**
  File registration API.
  @param category a category name (typically a plugin name)
  @param info an array of file info to register
  @param count the size of the info array
*/
typedef void (*register_file_v1_t)(const char *category,
                                   struct PSI_file_info_v1 *info, int count);

/**
  Create a file instrumentation for a created file.
  This method does not create the file itself, but is used to notify the
  instrumentation interface that a file was just created.
  @param key the file instrumentation key for this file
  @param name the file name
  @param file the file handle
*/
typedef void (*create_file_v1_t)(PSI_file_key key, const char *name, File file);

/**
  Get a file instrumentation locker, for opening or creating a file.
  @param state data storage for the locker
  @param key the file instrumentation key
  @param op the operation to perform
  @param name the file name
  @param identity a pointer representative of this file.
  @return a file locker, or NULL
*/
typedef struct PSI_file_locker *(*get_thread_file_name_locker_v1_t)(
    struct PSI_file_locker_state_v1 *state, PSI_file_key key,
    enum PSI_file_operation op, const char *name, const void *identity);

/**
  Get a file stream instrumentation locker.
  @param state data storage for the locker
  @param file the file stream to access
  @param op the operation to perform
  @return a file locker, or NULL
*/
typedef struct PSI_file_locker *(*get_thread_file_stream_locker_v1_t)(
    struct PSI_file_locker_state_v1 *state, struct PSI_file *file,
    enum PSI_file_operation op);

/**
  Get a file instrumentation locker.
  @param state data storage for the locker
  @param file the file descriptor to access
  @param op the operation to perform
  @return a file locker, or NULL
*/
typedef struct PSI_file_locker *(*get_thread_file_descriptor_locker_v1_t)(
    struct PSI_file_locker_state_v1 *state, File file,
    enum PSI_file_operation op);

/**
  Start a file instrumentation open operation.
  @param locker the file locker
  @param src_file the source file name
  @param src_line the source line number
*/
typedef void (*start_file_open_wait_v1_t)(struct PSI_file_locker *locker,
                                          const char *src_file,
                                          unsigned int src_line);

/**
  End a file instrumentation open operation, for file streams.
  @param locker the file locker.
  @param result the opened file (NULL indicates failure, non NULL success).
  @return an instrumented file handle
*/
typedef struct PSI_file *(*end_file_open_wait_v1_t)(
    struct PSI_file_locker *locker, void *result);

/**
  End a file instrumentation open operation, for non stream files.
  @param locker the file locker.
  @param file the file number assigned by open() or create() for this file.
*/
typedef void (*end_file_open_wait_and_bind_to_descriptor_v1_t)(
    struct PSI_file_locker *locker, File file);

/**
  End a file instrumentation open operation, for non stream temporary files.
  @param locker the file locker.
  @param file the file number assigned by open() or create() for this file.
  @param filename the file name generated during temporary file creation.
*/
typedef void (*end_temp_file_open_wait_and_bind_to_descriptor_v1_t)(
    struct PSI_file_locker *locker, File file, const char *filename);

/**
  Record a file instrumentation start event.
  @param locker a file locker for the running thread
  @param count the number of bytes requested, or 0 if not applicable
  @param src_file the source file name
  @param src_line the source line number
*/
typedef void (*start_file_wait_v1_t)(struct PSI_file_locker *locker,
                                     size_t count, const char *src_file,
                                     unsigned int src_line);

/**
  Record a file instrumentation end event.
  Note that for file close operations, the instrumented file handle
  associated with the file (which was provided to obtain a locker)
  is invalid after this call.
  @param locker a file locker for the running thread
  @param count the number of bytes actually used in the operation,
  or 0 if not applicable, or -1 if the operation failed
  @sa get_thread_file_name_locker
  @sa get_thread_file_stream_locker
  @sa get_thread_file_descriptor_locker
*/
typedef void (*end_file_wait_v1_t)(struct PSI_file_locker *locker,
                                   size_t count);

/**
  Start a file instrumentation close operation.
  @param locker the file locker
  @param src_file the source file name
  @param src_line the source line number
*/
typedef void (*start_file_close_wait_v1_t)(struct PSI_file_locker *locker,
                                           const char *src_file,
                                           unsigned int src_line);

/**
  End a file instrumentation close operation.
  @param locker the file locker.
  @param rc the close operation return code (0 for success).
*/
typedef void (*end_file_close_wait_v1_t)(struct PSI_file_locker *locker,
                                         int rc);

/**
  Record a file instrumentation start event.
  @param locker a file locker for the running thread
  @param count the number of bytes requested, or 0 if not applicable
  @param old_name name of the file to be renamed.
  @param new_name name of the file after rename.
  @param src_file the source file name
  @param src_line the source line number
*/
typedef void (*start_file_rename_wait_v1_t)(struct PSI_file_locker *locker,
                                            size_t count, const char *old_name,
                                            const char *new_name,
                                            const char *src_file,
                                            unsigned int src_line);
/**
 Rename a file instrumentation close operation.
 @param locker the file locker.
 @param old_name name of the file to be renamed.
 @param new_name name of the file after rename.
 @param rc the rename operation return code (0 for success).
*/
typedef void (*end_file_rename_wait_v1_t)(struct PSI_file_locker *locker,
                                          const char *old_name,
                                          const char *new_name, int rc);

typedef struct PSI_file_info_v1 PSI_file_info;
typedef struct PSI_file_locker_state_v1 PSI_file_locker_state;

/** @} (end of group psi_abi_file) */

#endif /* COMPONENTS_SERVICES_PSI_FILE_BITS_H */
