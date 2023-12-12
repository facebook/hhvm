<?hh
function my_error_handler($errno, $errmsg, $filename, $linenum, $vars)
:mixed{
	echo "$errno: $errmsg\n";
}
function foo($haystk, $needle, $offset, $to_charset = false, $from_charset = false)
:mixed{
	if ($from_charset !== false) {
		$haystk = iconv($from_charset, $to_charset, $haystk);
	}
	var_dump(strpos($haystk, $needle, $offset));
	if ($to_charset !== false) {
		var_dump(iconv_strpos($haystk, $needle, $offset, $to_charset));
	} else {
		var_dump(iconv_strpos($haystk, $needle, $offset));
	}
}

<<__EntryPoint>>
function main_entry(): void {
  set_error_handler(my_error_handler<>);
  foo("abecdbcdabef", "bcd", -1);
  foo("abecdbcdabef", "bcd", 100000);
  foo("abcabcabcdabcababcdabc", "bcd", 0);
  foo("abcabcabcdabcababcdabc", "bcd", 10);
  foo(str_repeat("abcab", 60)."abcdb".str_repeat("adabc", 60), "abcd", 0);
  foo(str_repeat("\xa4\xa2\xa4\xa4\xa4\xa6\xa4\xa8\xa4\xaa", 30)."\xa4\xa4\xa4\xa6\xa4\xaa\xa4\xa8\xa4\xa2".str_repeat("\xa4\xa2\xa4\xa4\xa4\xa8\xa4\xaa\xa4\xa6", 30), "\xa4\xa6\xa4\xaa", 0, "EUC-JP");
  $str = str_repeat("\xa4\xa2\xa4\xa4\xa4\xa6\xa4\xa8\xa4\xaa", 60).'$'.str_repeat("\xa4\xa2\xa4\xa4\xa4\xa8\xa4\xaa\xa4\xa6", 60);
  foo($str, '$', 0, "ISO-2022-JP", "EUC-JP");

  var_dump(iconv_strpos("string", ""));
  var_dump(iconv_strpos("", "string"));
}
