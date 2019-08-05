<?hh <<__EntryPoint>> function main(): void {
$str = "A\xc2\xa3BC";
  preg_match_all_with_matches('/\S\S/u', $str, &$m);
  var_dump($m);
  preg_match_all_with_matches('/\S{2}/u', $str, &$m);
  var_dump($m);

  $str = "A\xe2\x82\xac ";
  preg_match_all_with_matches('/\W\W/u', $str, &$m);
  var_dump($m);
  preg_match_all_with_matches('/\W{2}/u', $str, &$m);
  var_dump($m);
}
