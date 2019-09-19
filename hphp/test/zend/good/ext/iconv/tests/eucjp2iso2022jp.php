<?hh
/* include('test.inc'); */
/* charset=EUC-JP */

function hexdump($str) {
	$len = strlen($str);
	for ($i = 0; $i < $len; ++$i) {
		printf("%02x", ord($str{$i}));
	}
	print "\n";
}
<<__EntryPoint>>
function main_entry(): void {

  $str = str_repeat("���ܸ�ƥ����Ȥ� English text", 30);
  $str .= "���ܸ�";

  echo hexdump(iconv("EUC-JP", "ISO-2022-JP", $str));
}
