<?hh // partial

namespace {

/**
 * Close a bzip2 file
 *
 * @param resource $bz - The file pointer. It must be valid and must point to a
 *                       file successfully opened by bzopen().
 *
 * @return bool - Returns TRUE on success or FALSE on failure.
 */
<<__Native>>
function bzclose(resource $bz): bool;

/**
 * Compress a string into bzip2 encoded data
 *
 * @param string $source  - The string to compress.
 * @param int $blocksize  - Specifies the blocksize used during compression and
 *                          should be a number from 1 to 9 with 9 giving the
 *                          best compression, but using more resources to do so.
 * @param int $workfactor - Controls how the compression phase behaves when
 *                          presented with worst case, highly repetitive,
 *                          input data. The value can be between 0 and 250 with
 *                          0 being a special case.
 *                          Regardless of the $workfactor, the generated output
 *                          is the same.
 *
 * @return mixed - The compressed string, or an error number if an error
 *                 occurred.
 */
<<__Native, __IsFoldable>>
function bzcompress(string $source, int $blocksize = 4,
                    int $workfactor = 0)[]: mixed;

/**
 * Decompresses bzip2 encoded data
 *
 * @param string $source - The string to decompress.
 * @param int $small     - If TRUE, an alternative decompression algorithm will
 *                         be used which uses less memory (the maximum memory
 *                         requirement drops to around 2300K) but works at
 *                         roughly half the speed.
 *
 * @return mixed - The decompressed string, or an error number if an error
 *                 occurred.
 */
<<__Native, __IsFoldable>>
function bzdecompress(string $source, int $small = 0)[]: mixed;

/**
 * Returns a bzip2 error number
 *
 * @param resource $bz - The file pointer. It must be valid and must point to a
 *                       file successfully opened by bzopen().
 *
 * @return int - Returns the error number as an integer.
 */
<<__Native>>
function bzerrno(resource $bz): mixed;

/**
 * Returns the bzip2 error number and error string in an array
 *
 * @param resource $bz - The file pointer. It must be valid and must point to a
 *                       file successfully opened by bzopen().
 *
 * @return array - Returns an associative array, with the error code in the
 *                 errno entry, and the error message in the errstr entry.
 */
<<__Native>>
function bzerror(resource $bz): mixed;

/**
 * Returns a bzip2 error string
 *
 * @param resource $bz - The file pointer. It must be valid and must point to a
 *                       file successfully opened by bzopen().
 *
 * @return string - Returns a string containing the error message.
 */
<<__Native>>
function bzerrstr(resource $bz): mixed;

/**
 * Force a write of all buffered data
 *
 * @param resource $bz - The file pointer. It must be valid and must point to a
 *                       file successfully opened by bzopen().
 *
 * @return bool - Returns TRUE on success or FALSE on failure.
 */
<<__Native>>
function bzflush(resource $bz): bool;

/**
 * Opens a bzip2 compressed file
 *
 * @param string $filename - The name of the file to open.
 * @param string $mode     - Similar to the fopen() function, only 'r' (read)
 *                           and 'w' (write) are supported. Everything else will
 *                           cause bzopen to return FALSE.
 *
 * @return mixed - If the open fails, bzopen() returns FALSE, otherwise it
 *                 returns a pointer to the newly opened file.
 */
<<__Native>>
function bzopen(mixed $filename, string $mode): mixed;

/**
 * Binary safe bzip2 file read
 *
 * @param resource $bz - The file pointer. It must be valid and must point to a
 *                       file successfully opened by bzopen().
 * @param int $length  - If not specified, bzread() will read 1024
 *                       (uncompressed) bytes at a time. A maximum of 8192
 *                       uncompressed bytes will be read at a time.
 *
 * @return mixed - Returns the uncompressed data, or FALSE on error.
 */
<<__Native>>
function bzread(resource $bz, int $length = 1024): mixed;

/**
 * Binary safe bzip2 file write
 *
 * @param resource $bz - The file pointer. It must be valid and must point to a
 *                       file successfully opened by bzopen().
 * @param string $data - The written data.
 * @param int $length  - If supplied, writing will stop after length
 *                       (uncompressed) bytes have been written or the end of
 *                       data is reached, whichever comes first.
 *
 * @return mixed - Returns the number of bytes written, or FALSE on error.
 */
<<__Native>>
function bzwrite(resource $bz, string $data, int $length = 0): mixed;

} // root namespace

/*
 * Not a public API
 */
namespace __SystemLib {
<<__NativeData>>
class ChunkedBunzipper {
  <<__Native>>
  public function eof(): bool;

  <<__Native>>
  public function inflateChunk(string $chunk): string;

  <<__Native>>
  public function close(): void;
}
}
