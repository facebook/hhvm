<?hh
/*

print  "2: ". mb_strwidth($euc_jp, 'EUC-JP') . "\n";
print  "3: ". mb_strwidth($euc_jp, 'EUC-JP') . "\n";
// Note: Did not start form -22 offset. Staring from 0.
print  "4: ". mb_strwidth($euc_jp, 'EUC-JP') . "\n";

$str = mb_strwidth($euc_jp, 100, -10,'...','EUC-JP');
($str === "") ? print "5 OK\n" : print "NG: $str\n";

$str = mb_strwidth($euc_jp, -100, 10,'...','EUC-JP');
($str !== "") ?	print "6 OK: $str\n" : print "NG: $str\n";
*/
<<__EntryPoint>>
function main_entry(): void {
  // TODO: Add more encoding, strings.....

  ini_set('include_path', dirname(__FILE__));
  include_once('common.inc');

  // EUC-JP
  $euc_jp = "0123\xa4\xb3\xa4\xce\xca\xb8\xbb\xfa\xce\xf3\xa4\xcf\xc6\xfc\xcb\xdc\xb8\xec\xa4\xc7\xa4\xb9\xa1\xa3EUC-JP\xa4\xf2\xbb\xc8\xa4\xc3\xa4\xc6\xa4\xa4\xa4\xde\xa4\xb9\xa1\xa3\xc6\xfc\xcb\xdc\xb8\xec\xa4\xcf\xcc\xcc\xc5\xdd\xbd\xad\xa4\xa4\xa1\xa3";

  print  "1: ". mb_strwidth($euc_jp, 'EUC-JP') . "\n";
}
