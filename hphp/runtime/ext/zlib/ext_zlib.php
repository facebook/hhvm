<?hh // partial

/**
 * Close an open gz-file pointer
 *
 * @param resource $zp - The gz-file pointer. It must be valid, and must
 *   point to a file successfully opened by gzopen().
 *
 * @return bool -
 */
<<__Native>>
function gzclose(resource $zp): bool;

/**
 * ( excerpt from http://php.net/manual/en/function.zlib-encode.php )
 *
 * Compress data with the specified encoding. Warning: This function is
 * currently not documented; only its argument list is available.
 *
 * @data       mixed
 * @encoding   mixed
 * @level      mixed
 */
<<__Native, __IsFoldable, __Rx>>
function zlib_encode(string $data, int $encoding, int $level = -1): mixed;

/**
 * ( excerpt from http://php.net/manual/en/function.zlib-decode.php )
 *
 * Uncompress any raw/gzip/zlib encoded data. Warning: This function is
 * currently not documented; only its argument list is available.
 *
 * @data       mixed
 * @max_decoded_len
 *             mixed
 */
<<__Native, __IsFoldable, __Rx>>
function zlib_decode(string $data, int $max_len = 0): mixed;

/**
 * Compress a string
 *
 * @param string $data - The data to compress.
 * @param int $level - The level of compression. Can be given as 0 for no
 *   compression up to 9 for maximum compression.   If -1 is used, the
 *   default compression of the zlib library is used which is 6.
 *
 * @return string - The compressed string or FALSE if an error occurred.
 */
<<__Native, __IsFoldable, __Rx>>
function gzcompress(string $data, int $level = -1): mixed;

/**
 * Decodes a gzip compressed string
 *
 * @param string $data - The data to decode, encoded by gzencode().
 * @param int $length - The maximum length of data to decode.
 *
 * @return string - The decoded string, or FALSE if an error occurred.
 */
<<__Native, __IsFoldable, __Rx>>
function gzdecode(string $data,
                  int $length = 0): mixed;

/**
 * Deflate a string
 *
 * @param string $data - The data to deflate.
 * @param int $level - The level of compression. Can be given as 0 for no
 *   compression up to 9 for maximum compression. If not given, the default
 *   compression level will be the default compression level of the zlib
 *   library.
 *
 * @return string - The deflated string or FALSE if an error occurred.
 */
<<__Native, __IsFoldable, __Rx>>
function gzdeflate(string $data, int $level = -1): mixed;

/**
 * Create a gzip compressed string
 *
 * @param string $data - The data to encode.
 * @param int $level - The level of compression. Can be given as 0 for no
 *   compression up to 9 for maximum compression. If not given, the default
 *   compression level will be the default compression level of the zlib
 *   library.
 * @param int $encoding_mode - The encoding mode. Can be FORCE_GZIP (the
 *   default) or FORCE_DEFLATE.   Prior to PHP 5.4.0, using FORCE_DEFLATE
 *   results in a standard zlib deflated string (inclusive zlib headers)
 *   after a gzip file header but without the trailing crc32 checksum.   In
 *   PHP 5.4.0 and later, FORCE_DEFLATE generates RFC 1950 compliant
 *   output, consisting of a zlib header, the deflated data, and an Adler
 *   checksum.
 *
 * @return string - The encoded string, or FALSE if an error occurred.
 */
<<__Native, __IsFoldable, __Rx>>
function gzencode(string $data,
                  int $level = -1,
                  int $encoding_mode = FORCE_GZIP): mixed;

/**
 * Test for  on a gz-file pointer
 *
 * @param resource $zp - The gz-file pointer. It must be valid, and must
 *   point to a file successfully opened by gzopen().
 *
 * @return bool - Returns TRUE if the gz-file pointer is at EOF or an
 *   error occurs; otherwise returns FALSE.
 */
<<__Native>>
function gzeof(resource $zp): bool;

/**
 * Read entire gz-file into an array
 *
 * @param string $filename - The file name.
 * @param int $use_include_path - You can set this optional parameter to
 *   1, if you want to search for the file in the include_path too.
 *
 * @return array - An array containing the file, one line per cell.
 */
<<__Native>>
function gzfile(string $filename,
                int $use_include_path = 0): mixed;

/**
 * Get character from gz-file pointer
 *
 * @param resource $zp - The gz-file pointer. It must be valid, and must
 *   point to a file successfully opened by gzopen().
 *
 * @return string - The uncompressed character or FALSE on EOF (unlike
 *   gzeof()).
 */
<<__Native>>
function gzgetc(resource $zp): mixed;

/**
 * Get line from file pointer
 *
 * @param resource $zp - The gz-file pointer. It must be valid, and must
 *   point to a file successfully opened by gzopen().
 * @param int $length - The length of data to get.
 *
 * @return string - The uncompressed string, or FALSE on error.
 */
<<__Native>>
function gzgets(resource $zp,
                int $length = 0): mixed;

/**
 * Get line from gz-file pointer and strip HTML tags
 *
 *
 * @param resource $zp - The gz-file pointer. It must be valid, and must
 *   point to a file successfully opened by gzopen().
 * @param int $length - The length of data to get.
 * @param string $allowable_tags - You can use this optional parameter to
 *   specify tags which should not be stripped.
 *
 * @return string - The uncompressed and striped string, or FALSE on
 *   error.
 */
<<__Native>>
function gzgetss(resource $zp,
                 int $length = 0,
                 string $allowable_tags = ''): mixed;

/**
 * Inflate a deflated string
 *
 * @param string $data - The data compressed by gzdeflate().
 * @param int $length - The maximum length of data to decode.
 *
 * @return string - The original uncompressed data or FALSE on error.
 *   The function will return an error if the uncompressed data is more
 *   than 32768 times the length of the compressed input data or more than
 *   the optional parameter length.
 */
<<__Native, __IsFoldable, __Rx>>
function gzinflate(string $data, int $length = 0): mixed;

/**
 * Open gz-file
 *
 * @param string $filename - The file name.
 * @param string $mode - As in fopen() (rb or wb) but can also include a
 *   compression level (wb9) or a strategy: f for filtered data as in wb6f,
 *   h for Huffman only compression as in wb1h. (See the description of
 *   deflateInit2 in zlib.h for more information about the strategy
 *   parameter.)
 * @param int $use_include_path - You can set this optional parameter to
 *   1, if you want to search for the file in the include_path too.
 *
 * @return resource - Returns a file pointer to the file opened, after
 *   that, everything you read from this file descriptor will be
 *   transparently decompressed and what you write gets compressed.   If
 *   the open fails, the function returns FALSE.
 */
<<__Native>>
function gzopen(string $filename,
                string $mode,
                int $use_include_path = 0): mixed;

/**
 * Output all remaining data on a gz-file pointer
 *
 *
 * @param resource $zp - The gz-file pointer. It must be valid, and must
 *   point to a file successfully opened by gzopen().
 *
 * @return int - The number of uncompressed characters read from gz and
 *   passed through to the input, or FALSE on error.
 */
<<__Native>>
function gzpassthru(resource $zp): mixed;

/**
 * Alias of gzwrite()
 */
function gzputs(resource $zp, string $string, int $length = 0): mixed {
  return gzwrite($zp, $string, $length);
}

/**
 * Binary-safe gz-file read
 *
 * @param resource $zp - The gz-file pointer. It must be valid, and must
 *   point to a file successfully opened by gzopen().
 * @param int $length - The number of bytes to read.
 *
 * @return string - The data that have been read.
 */
<<__Native>>
function gzread(resource $zp,
                int $length = 0): mixed;

/**
 * Rewind the position of a gz-file pointer
 *
 * @param resource $zp - The gz-file pointer. It must be valid, and must
 *   point to a file successfully opened by gzopen().
 *
 * @return bool -
 */
<<__Native>>
function gzrewind(resource $zp): bool;

/**
 * Seek on a gz-file pointer
 *
 * @param resource $zp - The gz-file pointer. It must be valid, and must
 *   point to a file successfully opened by gzopen().
 * @param int $offset - The seeked offset.
 * @param int $whence - whence values are:  SEEK_SET - Set position equal
 *   to offset bytes. SEEK_CUR - Set position to current location plus
 *   offset.    If whence is not specified, it is assumed to be SEEK_SET.
 *
 * @return int - Upon success, returns 0; otherwise, returns -1. Note
 *   that seeking past EOF is not considered an error.
 */
<<__Native>>
function gzseek(resource $zp,
                int $offset,
                int $whence = SEEK_SET): mixed;

/**
 * Tell gz-file pointer read/write position
 *
 * @param resource $zp - The gz-file pointer. It must be valid, and must
 *   point to a file successfully opened by gzopen().
 *
 * @return int - The position of the file pointer or FALSE if an error
 *   occurs.
 */
<<__Native>>
function gztell(resource $zp): mixed;

/**
 * Uncompress a compressed string
 *
 * @param string $data - The data compressed by gzcompress().
 * @param int $length - The maximum length of data to decode.
 *
 * @return string - The original uncompressed data or FALSE on error.
 *   The function will return an error if the uncompressed data is more
 *   than 32768 times the length of the compressed input data or more than
 *   the optional parameter length.
 */
<<__Native>>
function gzuncompress(string $data, int $length = 0): mixed;

/**
 * Binary-safe gz-file write
 *
 * @param resource $zp - The gz-file pointer. It must be valid, and must
 *   point to a file successfully opened by gzopen().
 * @param string $string - The string to write.
 * @param int $length - The number of uncompressed bytes to write. If
 *   supplied, writing will stop after length (uncompressed) bytes have
 *   been written or the end of string is reached, whichever comes first.
 *    Note that if the length argument is given, then the
 *   magic_quotes_runtime configuration option will be ignored and no
 *   slashes will be stripped from string.
 *
 * @return int - Returns the number of (uncompressed) bytes written to
 *   the given gz-file stream.
 */
<<__Native>>
function gzwrite(resource $zp,
                 string $string,
                 int $length = 0): mixed;

/**
 * Output a gz-file
 *
 * @param string $filename - The file name. This file will be opened from
 *   the filesystem and its contents written to standard output.
 * @param int $use_include_path - You can set this optional parameter to
 *   1, if you want to search for the file in the include_path too.
 *
 * @return int - Returns the number of (uncompressed) bytes read from the
 *   file. If an error occurs, FALSE is returned and unless the function
 *   was called as @readgzfile, an error message is printed.
 */
<<__Native>>
function readgzfile(string $filename,
                    int $use_include_path = 0): mixed;

/**
 * This function compresses the given string using the nzlib data format, which
 * is primarily used for compressing and uncompressing memcache values.
 *
 * @param string $uncompressed - The uncompressed data
 *
 * @return string - The compressed data, or FALSE on error
 */
<<__Native, __HipHopSpecific, __IsFoldable, __Rx>>
function nzcompress(string $uncompressed): mixed;

/**
 * This function uncompresses the given string given that it is in the nzlib
 * data format, which is primarily used for compressing and uncompressing
 * memcache values
 *
 * @param string $compressed - The data compressed by nzcompress().
 *
 * @return string - The uncompressed data or FALSE on error
 */
<<__Native, __HipHopSpecific, __IsFoldable, __Rx>>
function nzuncompress(string $compressed): mixed;

/**
 * Implementation detail for zlib.inflate stream filter.
 *
 * Not a public API
 */
namespace __SystemLib {
<<__NativeData("__SystemLib\\ChunkedInflator")>>
class ChunkedInflator {
  <<__Native>>
  function eof(): bool;

  <<__Native>>
  function inflateChunk(string $chunk): string;
}

<<__NativeData("__SystemLib\\ChunkedGunzipper")>>
class ChunkedGunzipper {
  <<__Native>>
  function eof(): bool;

  <<__Native>>
  function inflateChunk(string $chunk): string;
}
}
