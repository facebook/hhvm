<?php

/**
 *
 * Provide compatibility for renamed lz4 function -
 * lz4compress is now lz4_compress
 * 
 */
function lz4compress(string $uncompressed) {
  trigger_error("lz4compress is now depreciated in favour of lz4_compress", 
                 E_USER_WARNING);
  return lz4_compress($uncompressed);
}

/**
 *
 * Provide compatibility for renamed lz4 function -
 * lz4hccompress is now lz4_hccompress
 * 
 */
function lz4hccompress(string $uncompressed) {
  trigger_error("lz4hccompress is now depreciated in favour of lz4_hccompress", 
                E_USER_WARNING);
  return lz4_hccompress($uncompressed);
}

/**
 *
 * Provide compatibility for renamed lz4 function -
 * lz4uncompress is now lz4_uncompress
 * 
 */
function lz4uncompress(string $compressed) {
  trigger_error("lz4uncompress is now depreciated in favour of lz4_uncompress", 
                E_USER_WARNING);
  return lz4_uncompress($compressed);
}
