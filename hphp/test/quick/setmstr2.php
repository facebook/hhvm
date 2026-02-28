<?hh
function bar($a) :mixed{}
<<__EntryPoint>> function baz(): void {
  $cipher = 'abcdefghij';
  $pos = 4;
  $random_byte = chr(25);
  $cipher[$pos] = $random_byte;
  var_dump(ord($random_byte));
}
