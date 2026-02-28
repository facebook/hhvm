<?hh

class Tester {
    function Hello() :mixed{
        echo "Hello\n";
    }
}
<<__EntryPoint>>
function entrypoint_gzencode_error1(): void {
  /* Prototype  : string gzencode  ( string $data  [, int $level  [, int $encoding_mode  ]] )
   * Description: Gzip-compress a string
   * Source code: ext/zlib/zlib.c
   * Alias to functions:
   */

  /*
   * Test error cases for gzencode
   */

  echo "*** Testing gzencode() : error conditions ***\n";

  // Zero arguments
  echo "\n-- Testing gzencode() function with Zero arguments --\n";
  try { var_dump( gzencode() ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

  //Test gzencode with one more than the expected number of arguments
  echo "\n-- Testing gzencode() function with more than expected no. of arguments --\n";
  $data = 'string_val';
  $level = 2;
  $encoding_mode = FORCE_DEFLATE;
  $extra_arg = 10;
  try { var_dump( gzencode($data, $level, $encoding_mode, $extra_arg) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

  echo "\n-- Testing with incorrect compression level --\n";
  $bad_level = 99;
  var_dump(gzencode($data, $bad_level));

  echo "\n-- Testing with incorrect encoding_mode --\n";
  $bad_mode = 99;
  var_dump(gzencode($data, $level, $bad_mode));
  echo "===Done===";
}
