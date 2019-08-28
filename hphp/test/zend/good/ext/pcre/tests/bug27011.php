<?hh
<<__EntryPoint>> function main(): void {
  $matches = null;
  var_dump(preg_match_all_with_matches(
    '|(\w+)://([^\s"<]*[\w+#?/&=])|',
    "This is a text string",
    inout $matches,
    PREG_SET_ORDER,
  ));
  var_dump($matches);
}
