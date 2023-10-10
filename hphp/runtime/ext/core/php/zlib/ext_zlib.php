<?hh

/**
 *
 * This is a wrapper function as lz4compress is now lz4_compress
 *
 * https://github.com/facebook/hhvm/pull/3169 - 11/07/2014
 *
 */

function lz4compress(string $uncompressed) {
  return lz4_compress($uncompressed);
}

/**
 *
 * This is a wrapper function as lz4hccompress is now lz4_hccompress
 *
 * https://github.com/facebook/hhvm/pull/3169 - 11/07/2014
 *
 */

function lz4hccompress(string $uncompressed) {
  return lz4_hccompress($uncompressed);
}

/**
 *
 * This is a wrapper function as lz4uncompress is now lz4_uncompress
 *
 * https://github.com/facebook/hhvm/pull/3169 - 11/07/2014
 *
 */

function lz4uncompress(string $compressed) {
  return lz4_uncompress($compressed);
}
