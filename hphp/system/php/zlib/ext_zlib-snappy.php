<?php

/**
 *
 * This is a wrapper function as sncompress is now snappy_compress
 *
 * https://github.com/facebook/hhvm/pull/3258 - 23/07/2014
 *
 */
<<__HipHopSpecific>>
function sncompress(string $data) {
  trigger_error("sncompress is now depreciated in favour of snappy_compress",
                 E_USER_WARNING);
  return snappy_compress($uncompressed);
}

/**
 *
 * This is a wrapper function as snuncompress is now snappy_uncompress
 *
 * https://github.com/facebook/hhvm/pull/3258 - 23/07/2014
 *
 */
<<__HipHopSpecific>>
function snuncompress(string $data) {
  trigger_error("snuncompress is now depreciated in favour of snappy_uncompress",
                E_USER_WARNING);
  return snappy_uncompress($compressed);
}

