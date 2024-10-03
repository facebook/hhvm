<?hh

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

  echo "===DONE===\n";
}
