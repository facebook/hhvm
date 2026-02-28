<?hh
function hexdump($str) :mixed{
	$len = strlen($str);
	for ($i = 0; $i < $len; ++$i) {
		printf("%02x", ord($str[$i]));
	}
	print "\n";
}

function foo($str, $offset, $len, $charset) :mixed{
	hexdump(substr($str, $offset, $len));
	hexdump(iconv_substr($str, $offset, $len, $charset));
}

function bar($str, $offset, $len = false) :mixed{
	if (is_bool($len)) {
		var_dump(substr($str, $offset));
		var_dump(iconv_substr($str, $offset));
	} else {
		var_dump(substr($str, $offset, $len));
		var_dump(iconv_substr($str, $offset, $len));
	}
}
<<__EntryPoint>>
function main_entry(): void {

  foo("abcdefghijklmnopqrstuvwxyz", 5, 7, "ASCII");
  foo("\xa4\xa2\xa4\xa4\xa4\xa6\xa4\xa8\xa4\xaa\xa4\xab\xa4\xad\xa4\xaf\xa4\xb1\xa4\xb3\xa4\xb5\xa4\xb7\xa4\xb9", 5, 7, "EUC-JP");
  bar("This is a test", 100000);
  bar("This is a test", 0, 100000);
  bar("This is a test", -3);
  bar("This is a test", 0, -9);
  bar("This is a test", 0, -100000);
  bar("This is a test", -9, -100000);
  var_dump(iconv("ISO-2022-JP", "EUC-JP", iconv_substr(iconv("EUC-JP", "ISO-2022-JP", "\xa4\xb3\xa4\xf3\xa4\xcb\xa4\xc1\xa4\xcf ISO-2022-JP"), 3, 8, "ISO-2022-JP")));
}
