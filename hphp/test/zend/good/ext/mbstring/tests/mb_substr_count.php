<?hh
<<__EntryPoint>>
function main_entry(): void {
  	mb_internal_encoding("EUC-JP");
  	var_dump(@mb_substr_count("", ""));
  	var_dump(@mb_substr_count("\xa4\xa2", ""));
  	var_dump(@mb_substr_count("", "\xa4\xa2"));
  	var_dump(@mb_substr_count("", "\xa4\xa2"));
  	var_dump(@mb_substr_count("", chr(0)));

  	$a = str_repeat("abcacba", 100);
  	var_dump(@mb_substr_count($a, "bca"));

  	$a = str_repeat("\xa4\xa2\xa4\xa4\xa4\xa6\xa4\xa2\xa4\xa6\xa4\xa4\xa4\xa2", 100);
  	$b = "\xa4\xa4\xa4\xa6\xa4\xa2";
  	var_dump(@mb_substr_count($a, $b));

  	$to_enc = "UTF-8";
  	var_dump(@mb_substr_count(mb_convert_encoding($a, $to_enc),
  	                          mb_convert_encoding($b, $to_enc), $to_enc));

  	$to_enc = "Shift_JIS";
  	var_dump(@mb_substr_count(mb_convert_encoding($a, $to_enc),
  	                          mb_convert_encoding($b, $to_enc), $to_enc));

  	$a = str_repeat("abcacbabca", 100);
  	var_dump(@mb_substr_count($a, "bca"));
}
