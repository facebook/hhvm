<?hh

<<__EntryPoint>>
function main_parse_cookies(): mixed {
  // duplicate cookies should take the first value
  var_dump(HH\parse_cookies('foo=bar; foo=1;'));

  // array behavior
  var_dump(HH\parse_cookies('foo[0]=bar; foo[1]=baz'));

  // handle ints and lack of spaces
  var_dump(HH\parse_cookies('foo=1;bar=2'));

  // empty string
  var_dump(HH\parse_cookies(''));

  // strtok_r is destructive, avoid side effects
  $header = 'foo=1; bar=2; baz=3';
  var_dump(HH\parse_cookies($header));
  var_dump($header);
}
