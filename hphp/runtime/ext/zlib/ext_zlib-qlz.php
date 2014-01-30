<?hh

/**
 * This function compress the given string using the QuickLZ data format.
 *
 * For details on the QuickLZ compression algorithm go to
 * http://www.quicklz.com/(RFC 1950).
 *
 * @param string $data - The data to compress
 * @param int $level   - The level of compression. Can be given as 1, 2 or 3.
 *
 * @return string - The compressed string or FALSE if an error occurred
 */
<<__Native, __HipHopSpecific>>
function qlzcompress(string $data, int $level = 1): mixed;

/**
 * This function uncompress a compressed string
 *
 * @param string $data - The data to compressed by qlzcompress()
 * @param int $level   - The level of compression. Can be given as 1, 2 or 3.
 *
 * @return string - The original uncompressed data or FALSE if an error occurred
 */
<<__Native, __HipHopSpecific>>
function qlzuncompress(string $data, int $level = 1): mixed;
