<?hh
<<__EntryPoint>>
function main_entry(): void {
  // TODO: Add more encoding

  ini_set('include_path', dirname(__FILE__));
  include_once('common.inc');

  // EUC-JP
  $euc_jp = '0123����ʸ��������ܸ�Ǥ���EUC-JP��ȤäƤ��ޤ������ܸ�����ݽ�����';

  print  "1: ". mb_strimwidth($euc_jp,  0, 15,'...','EUC-JP') . "\n";
  print  "2: ". mb_strimwidth($euc_jp,  0, 100,'...','EUC-JP') . "\n";
  print  "3: ". mb_strimwidth($euc_jp, 15, 100,'...','EUC-JP') . "\n";
  // Note: Did not start form -22 offset. Staring from 0.
  $str = mb_strimwidth($euc_jp,-22, 100,'...','EUC-JP');
  ($str === FALSE) ? print "4 OK\n" : print "NG: $str\n";

  $str = mb_strimwidth($euc_jp, 100, -10,'...','EUC-JP');
  ($str === FALSE) ? print "5 OK\n" : print "NG: $str\n";

  $str = mb_strimwidth($euc_jp, -100, 10,'...','EUC-JP');
  ($str === FALSE) ? print "6 OK\n" : print "NG: $str\n";
}
