<?hh
<<__EntryPoint>>
function entrypoint_mb_strlen(): void {
  // TODO: Add more encodings


  ini_set('include_path', dirname(__FILE__));
  include_once('common.inc');
  set_custom_error_handler();

  // restore detect_order to 'auto'
  mb_detect_order('auto');

  // Test string
  $euc_jp = "0123\xa4\xb3\xa4\xce\xca\xb8\xbb\xfa\xce\xf3\xa4\xcf\xc6\xfc\xcb\xdc\xb8\xec\xa4\xc7\xa4\xb9\xa1\xa3EUC-JP\xa4\xf2\xbb\xc8\xa4\xc3\xa4\xc6\xa4\xa4\xa4\xde\xa4\xb9\xa1\xa30123\xc6\xfc\xcb\xdc\xb8\xec\xa4\xcf\xcc\xcc\xc5\xdd\xbd\xad\xa4\xa4\xa1\xa3";
  $ascii  = 'abcdefghijklmnopqrstuvwxyz;]=#0123456789';

  // ASCII
  echo "== ASCII ==\n";
  print  mb_strlen($ascii,'ASCII') . "\n";
  print  strlen($ascii) . "\n";

  // EUC-JP
  echo "== EUC-JP ==\n";
  print  mb_strlen($euc_jp,'EUC-JP') . "\n";
  mb_internal_encoding('EUC-JP') || print("mb_internal_encoding() failed\n");
  print  strlen($euc_jp) . "\n";

  // SJIS
  echo "== SJIS ==\n";
  $sjis = mb_convert_encoding($euc_jp, 'SJIS','EUC-JP');
  print  mb_strlen($sjis,'SJIS') . "\n";
  mb_internal_encoding('SJIS') || print("mb_internal_encoding() failed\n");
  print  strlen($sjis) . "\n";

  // JIS
  // Note: either convert_encoding or strlen has problem
  echo "== JIS ==\n";
  $jis = mb_convert_encoding($euc_jp, 'JIS','EUC-JP');
  print  mb_strlen($jis,'JIS') . "\n";
  mb_internal_encoding('JIS') || print("mb_internal_encoding() failed\n");
  print  strlen($jis) . "\n"; 

  // UTF-8
  // Note: either convert_encoding or strlen has problem
  echo "== UTF-8 ==\n";
  $utf8 = mb_convert_encoding($euc_jp, 'UTF-8','EUC-JP');
  print  mb_strlen($utf8,'UTF-8') . "\n";
  mb_internal_encoding('UTF-8') || print("mb_internal_encoding() failed\n");
  print  strlen($utf8) . "\n";  


  // Wrong Parameters
  echo "== WRONG PARAMETERS ==\n";
  // Array
  // Note: PHP Warning, strlen() expects parameter 1 to be string, array given
  $r = null;
  try { $r = strlen(t_ary()); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
  echo (string)$r."\n";
  // Object
  // Note: PHP Warning, strlen() expects parameter 1 to be string, object given
  $r = null;
  try { $r = strlen(t_obj()); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
  echo (string)$r."\n";
  // Wrong encoding
  mb_internal_encoding('EUC-JP');
  $r = mb_strlen($euc_jp, 'BAD_NAME');
  echo (string)$r."\n";
}
