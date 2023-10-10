<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

/**
 * shmop_close() - http://php.net/function.shmop_close
 *
 * shmop_close() is used to close a shared memory block.
 *
 * @param int $shmid - The shared memory block identifier created by
 *                     shmop_open()
 */
<<__Native>>
function shmop_close(int $shmid): void;

/**
 * shmop_delete() - http://php.net/function.shmop_delete
 *
 * shmop_delete() is used to delete a shared memory block.
 *
 * @param int $shmid - The shared memory block identifier created by
 *                     shmop_open()
 *
 * @return bool - Returns TRUE on success or FALSE on failure.
 */
<<__Native>>
function shmop_delete(int $shmid): bool;

/**
 * shmop_open() - http://php.net/function.shmop_open
 *
 * shmop_open() can create or open a shared memory block.
 *
 * @param int $key - System's id for the shared memory block. Can be passed as a
 *                   decimal or hex.
 * @param string $flags - The flags that you can use:
 *                        "a" for access (sets SHM_RDONLY for shmat) use this
 *                          flag when you need to open an existing shared memory
 *                          segment for read only
 *
 *                        "c" for create (sets IPC_CREATE) use this flag when
 *                          you need to create a new shared memory segment or if
 *                          a segment with the same key exists, try to open it
 *                          for read and write
 *
 *                        "w" for read & write access use this flag when you
 *                          need to read and write to a shared memory segment,
 *                          use this flag in most cases.
 *
 *                        "n" create a new memory segment (sets
 *                          IPC_CREATE|IPC_EXCL) use this flag when you want to
 *                          create a new shared memory segment but if one
 *                          already exists with the same flag, fail. This is
 *                          useful for security purposes, using this you can
 *                          prevent race condition exploits.
 * @param int $mode - The permissions that you wish to assign to your memory
 *                    segment, those are the same as permission for a
 *                    file. Permissions need to be passed in octal form, like
 *                    for example 0644
 * @param int $size - The size of the shared memory block you wish to create in
 *                    bytes
 *
 * @return mixed - On success shmop_open() will return an id that you can use to
 *                 access the shared memory segment you've created. FALSE is
 *                 returned on failure.
 *
 * @note The 3rd and 4th should be entered as 0 if you are opening an existing
 *       memory segment.
 */
<<__Native>>
function shmop_open(int $key, string $flags, int $mode, int $size): mixed;

/**
 * shmop_read() - http://php.net/function.shmop_read
 *
 * shmop_read() will read a string from shared memory block.
 *
 * @param int $shmid - The shared memory block identifier created by
 *                     shmop_open()
 * @param int $start - Offset from which to start reading
 * @param int $count - The number of bytes to read
 *
 * @return mixed - Returns the data or FALSE on failure.
 */
<<__Native>>
function shmop_read(int $shmid, int $start, int $count): mixed;

/**
 * shmop_size() - http://php.net/function.shmop_size
 *
 * shmop_size() is used to get the size, in bytes of the shared memory block.
 *
 * @param int $shmid - The shared memory block identifier created by
 *                     shmop_open()
 *
 * @return int - Returns an int, which represents the number of bytes the shared
 *               memory block occupies.
 */
<<__Native>>
function shmop_size(int $shmid): int;

/**
 * shmop_write() - http://php.net/function.shmop_write
 *
 * shmop_write() will write a string into shared memory block.
 *
 * @param int $shmid - The shared memory block identifier created by
 *                     shmop_open()
 * @param string $data - A string to write into shared memory block
 * @param int $offset - Specifies where to start writing data inside the shared
 *                      memory segment.
 *
 * @return mixed - The size of the written data, or FALSE on failure.
 */
<<__Native>>
function shmop_write(int $shmid, string $data, int $offset): mixed;
