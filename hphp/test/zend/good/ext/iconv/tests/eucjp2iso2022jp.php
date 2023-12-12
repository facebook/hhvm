<?hh
/* include('test.inc'); */
/* charset=EUC-JP */

function hexdump($str) :mixed{
	$len = strlen($str);
	for ($i = 0; $i < $len; ++$i) {
		printf("%02x", ord($str{$i}));
	}
	print "\n";
}
<<__EntryPoint>>
function main_entry(): void {

  $str = str_repeat("\xc6\xfc\xcb\xdc\xb8\xec\xa5\xc6\xa5\xad\xa5\xb9\xa5\xc8\xa4\xc8 English text", 30);
  $str .= "\xc6\xfc\xcb\xdc\xb8\xec";

  echo hexdump(iconv("EUC-JP", "ISO-2022-JP", $str));
}
