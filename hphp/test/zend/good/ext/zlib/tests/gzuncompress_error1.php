<?hh

class Tester {
    function Hello() :mixed{
        echo "Hello\n";
    }
}
<<__EntryPoint>>
function entrypoint_gzuncompress_error1(): void {
  /* Prototype  : string gzuncompress(string data [, int length])
   * Description: Unzip a gzip-compressed string
   * Source code: ext/zlib/zlib.c
   * Alias to functions:
   */

  echo "*** Testing gzuncompress() : error conditions ***\n";

  // Zero arguments
  echo "\n-- Testing gzuncompress() function with Zero arguments --\n";
  try { var_dump( gzuncompress() ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

  //Test gzuncompress with one more than the expected number of arguments
  echo "\n-- Testing gzuncompress() function with more than expected no. of arguments --\n";
  $data = 'string_val';
  $length = 10;
  $extra_arg = 10;
  try { var_dump( gzuncompress($data, $length, $extra_arg) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

  echo "\n-- Testing with a buffer that is too small --\n";
  $short_len = strlen($data) - 1;
  $compressed = gzcompress($data);

  var_dump(gzuncompress($compressed, $short_len));

  echo "\n-- Testing with incorrect arguments --\n";
  var_dump(gzuncompress('123'));
  echo "===DONE===\n";
}
