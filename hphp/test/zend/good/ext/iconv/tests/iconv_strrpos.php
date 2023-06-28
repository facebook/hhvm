<?hh
function my_error_handler($errno, $errmsg, $filename, $linenum, $vars)
:mixed{
	echo "$errno: $errmsg\n";
}
function foo($haystk, $needle, $to_charset = false, $from_charset = false)
:mixed{
	if ($from_charset !== false) {
		$haystk = iconv($from_charset, $to_charset, $haystk);
	}
	if ($to_charset !== false) {
		var_dump(iconv_strlen($haystk, $to_charset));
		var_dump(iconv_strrpos($haystk, $needle, $to_charset));
	} else {
		var_dump(iconv_strlen($haystk));
		var_dump(iconv_strrpos($haystk, $needle));
	}
}

<<__EntryPoint>>
function main_entry(): void {
  set_error_handler(my_error_handler<>);
  foo("abecdbcdabcdef", "bcd");
  foo(str_repeat("abcab", 60)."abcdb".str_repeat("adabc", 60), "abcd");
  foo(str_repeat("あいうえお", 30)."いうおえあ".str_repeat("あいえおう", 30), "うお", "EUC-JP");

  for ($i = 0; $i <=6; ++$i) {
  	$str = str_repeat("あいうえお", 60).str_repeat('$', $i).str_repeat("あいえおう", 60);
  	foo($str, '$', "ISO-2022-JP", "EUC-JP");
  }

  var_dump(iconv_strrpos("string", ""));
  var_dump(iconv_strrpos("", "string"));
}
