<?hh

<<__EntryPoint>>
function test(): void {
  $obj = new stdClass();
  $test = vec[keyset[1, 2], $obj, $obj];
  $roundtrip = unserialize(serialize($test));
  var_dump($roundtrip);
  var_dump($roundtrip[1] === $roundtrip[2]);
}
