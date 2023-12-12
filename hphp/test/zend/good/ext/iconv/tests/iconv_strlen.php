<?hh
function foo($str, $charset) :mixed{
	var_dump(strlen($str));
	var_dump(iconv_strlen($str, $charset));
}
<<__EntryPoint>>
function main_entry(): void {

  foo("abc", "ASCII");
  foo("\xc6\xfc\xcb\xdc\xb8\xec EUC-JP", "EUC-JP");
}
