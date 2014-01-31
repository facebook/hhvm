<?hh

/**
 * This function compress the given string using the Snappy data format.
 *
 * For details on the Snappy compression algorithm go to
 * http://code.google.com/p/snappy/.
 *
 * @param string $data - The data to compress
 *
 * @return string - The compressed string or FALSE if an error occurred.
 */
<<__Native, __HipHopSpecific>>
function sncompress(string $data): mixed;

/**
 * This function uncompress a compressed string.
 *
 * @param string $data - The data compressed by sncompress()
 *
 * @return string - The decompressed string or FALSE if an error occurred.
 */
<<__Native, __HipHopSpecific>>
function snuncompress(string $data): mixed;
