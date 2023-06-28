<?hh
function foo($str, $charset) :mixed{
	var_dump(strlen($str));
	var_dump(iconv_strlen($str, $charset));
}
<<__EntryPoint>>
function main_entry(): void {

  foo("abc", "ASCII");
  foo("ฦüหÜธ์ EUC-JP", "EUC-JP");
}
