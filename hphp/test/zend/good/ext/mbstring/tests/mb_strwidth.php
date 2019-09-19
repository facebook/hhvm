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
  $euc_jp = '0123����ʸ��������ܸ�Ǥ���EUC-JP��ȤäƤ��ޤ������ܸ�����ݽ�����';

  print  "1: ". mb_strwidth($euc_jp, 'EUC-JP') . "\n";
}
