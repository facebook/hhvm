<?hh
<<__EntryPoint>>
function main_entry(): void {
  // TODO: Add more encodings
  ini_set('include_path','.');
  include_once('common.inc');

  // EUC-JP
  $euc_jp = b"0123\xa4\xb3\xa4\xce\xca\xb8\xbb\xfa\xce\xf3\xa4\xcf\xc6\xfc\xcb\xdc\xb8\xec\xa4\xc7\xa4\xb9\xa1\xa3EUC-JP\xa4\xf2\xbb\xc8\xa4\xc3\xa4\xc6\xa4\xa4\xa4\xde\xa4\xb9\xa1\xa3\xc6\xfc\xcb\xdc\xb8\xec\xa4\xcf\xcc\xcc\xc5\xdd\xbd\xad\xa4\xa4\xa1\xa3";

  print  "1: ". bin2hex(mb_substr($euc_jp,  10,  10,'EUC-JP')) . "\n";
  print  "2: ". bin2hex(mb_substr($euc_jp,   0, 100,'EUC-JP')) . "\n";

  $str = mb_substr($euc_jp, 100, 10,'EUC-JP');
  // Note: returns last character
  ($str === "") ? print "3 OK\n" : print "NG: ".bin2hex($str)."\n";

  $str = mb_substr($euc_jp, -100, 10,'EUC-JP');
  ($str !== "") ? print "4 OK: ".bin2hex($str)."\n" : print "NG: ".bin2hex($str)."\n";
}
