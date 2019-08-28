<?hh
<<__EntryPoint>> function main(): void {
  $m = null;
  var_dump(preg_match_all_with_matches('/\b/', "a'", inout $m, PREG_OFFSET_CAPTURE));
  var_dump($m);

var_dump(preg_split('/\b/', "a'"));
var_dump(preg_split('/\b/', "a'", -1, PREG_SPLIT_OFFSET_CAPTURE));
var_dump(preg_split('/\b/', "a'", -1, PREG_SPLIT_NO_EMPTY));
var_dump(preg_split('/\b/', "a'", -1, PREG_SPLIT_NO_EMPTY|PREG_SPLIT_OFFSET_CAPTURE));
}
