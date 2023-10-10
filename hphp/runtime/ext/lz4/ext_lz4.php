<?hh

/**
 * This function compresses the given string using the lz4lib data format, which
 * is primarily used for compressing and uncompressing memcache values.
 *
 * @param string $uncompressed - The uncompressed data
 *
 * @return string - The compressed data, or FALSE on error
 */
<<__Native, __IsFoldable>>
function lz4_compress(string $uncompressed, bool $high = false)[]: mixed;

/**
 * This function compresses the given string using the lz4lib data format, which
 * is primarily used for compressing and uncompressing memcache values.
 *
 * @param string $uncompressed - The uncompressed data
 *
 * @return string - The compressed data, or FALSE on error
 */
<<__IsFoldable>>
function lz4_hccompress(string $uncompressed)[]: mixed {
  return lz4_compress($uncompressed, true);
}

/**
 * This function uncompresses the given string given that it is in the lz4lib
 * data format, which is primarily used for compressing and uncompressing
 * memcache values
 *
 * @param string $compressed - The data compressed by lz4compress().
 *
 * @return string - The uncompressed data or FALSE on error
 */
<<__Native, __IsFoldable>>
function lz4_uncompress(string $compressed)[]: mixed;
