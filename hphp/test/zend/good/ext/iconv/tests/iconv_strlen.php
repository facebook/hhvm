<?hh
function foo($str, $charset) {
	var_dump(strlen($str));
	var_dump(iconv_strlen($str, $charset));
}
<<__EntryPoint>>
function main_entry(): void {

  foo("abc", "ASCII");
  foo("���ܸ� EUC-JP", "EUC-JP");
}
