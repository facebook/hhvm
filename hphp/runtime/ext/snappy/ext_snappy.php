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
<<__Native>>
function snappy_compress(string $data)[]: mixed;

/**
 * This function uncompress a compressed string.
 *
 * @param string $data - The data compressed by snappy_compress()
 *
 * @return string - The decompressed string or FALSE if an error occurred.
 */
<<__Native>>
function snappy_uncompress(string $data)[]: mixed;

/**
 * This is a wrapper function as sncompress is now snappy_compress
 *
 * https://github.com/facebook/hhvm/pull/3258 - 23/07/2014
 */
<<__Native>>
function sncompress(string $data)[]: mixed;

/**
 * This is a wrapper function as snuncompress is now snappy_uncompress
 *
 * https://github.com/facebook/hhvm/pull/3258 - 23/07/2014
 */
<<__Native>>
function snuncompress(string $data)[]: mixed;
