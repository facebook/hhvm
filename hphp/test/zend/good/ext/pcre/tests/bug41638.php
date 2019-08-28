<?hh <<__EntryPoint>> function main(): void {
$str = "repeater id='loopt' dataSrc=subject colums=2";

  $str_instead = null;
  preg_match_all_with_matches(
    "/(['\"])((.*(\\\\\\1)*)*)\\1/sU",
    $str,
    inout $str_instead,
  );
  print_r($str_instead);

// these two are from Magnus Holmgren (extracted from a pcre-dev mailing list post)
  preg_match_all_with_matches(
    "/(['\"])((?:\\\\\\1|.)*)\\1/sU",
    $str,
    inout $str_instead,
  );
  print_r($str_instead);

  preg_match_all_with_matches(
    "/(['\"])(.*)(?<!\\\\)\\1/sU",
    $str,
    inout $str_instead,
  );
  print_r($str_instead);
}
