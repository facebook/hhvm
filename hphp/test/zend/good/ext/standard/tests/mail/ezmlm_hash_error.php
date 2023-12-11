<?hh
class sample  {
}
<<__EntryPoint>>
function entrypoint_ezmlm_hash_error(): void {
  /* Prototype  : int ezmlm_hash  ( string $addr  )
   * Description: Calculate the hash value needed by EZMLM.
   * Source code: ext/standard/mail.c
   */

  echo "*** Testing ezmlm_hash() : error conditions ***\n";

  echo "\n-- Testing ezmlm_hash() function with fewer than expected no. of arguments --\n";
  try { var_dump( ezmlm_hash() ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

  echo "\n-- Testing ezmlm_hash() function with more than expected no. of arguments --\n";
  $extra_arg = 10;
  try { var_dump( ezmlm_hash("webmaster@example.com", $extra_arg) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

  echo "\n-- Testing ezmlm_hash() function with invalid input - ARRAY --\n";
  $array_arg = vec[1,2,3,4];
  $extra_arg = 10;
  try { var_dump( ezmlm_hash($array_arg) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

  echo "\n-- Testing ezmlm_hash() function with invalid input - OBJECT without 'cast_object' method --\n";

  $obj_arg = new sample();
  try { var_dump( ezmlm_hash($obj_arg) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

  echo "\n-- Testing ezmlm_hash() function with invalid input - RESOURCE --\n";
  $file_handle = fopen(__FILE__, "r");
  $extra_arg = 10;
  try { var_dump( ezmlm_hash($file_handle) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
  fclose($file_handle); 

  echo "===DONE===\n";
}
