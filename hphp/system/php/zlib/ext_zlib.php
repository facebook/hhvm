<?php

/**
 *
 * This is a wrapper function as lz4compress is now lz4_compress
 *
 * https://github.com/facebook/hhvm/pull/3169 - 11/07/2014
 *
 */
<<__HipHopSpecific>>
function lz4compress(string $uncompressed) {
  trigger_error("lz4compress is now depreciated in favour of lz4_compress",
                 E_USER_WARNING);
  return lz4_compress($uncompressed);
}

/**
 *
 * This is a wrapper function as lz4hccompress is now lz4_hccompress
 *
 * https://github.com/facebook/hhvm/pull/3169 - 11/07/2014
 *
 */
<<__HipHopSpecific>>
function lz4hccompress(string $uncompressed) {
  trigger_error("lz4hccompress is now depreciated in favour of lz4_hccompress",
                E_USER_WARNING);
  return lz4_hccompress($uncompressed);
}

/**
 *
 * This is a wrapper function as lz4uncompress is now lz4_uncompress
 *
 * https://github.com/facebook/hhvm/pull/3169 - 11/07/2014
 *
 */
<<__HipHopSpecific>>
function lz4uncompress(string $compressed) {
  trigger_error("lz4uncompress is now depreciated in favour of lz4_uncompress",
                E_USER_WARNING);
  return lz4_uncompress($compressed);
}
