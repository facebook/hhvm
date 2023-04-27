<?hh

<<__Memoize(#KeyedByIC)>>
function memo1() {
  return $x;
}

<<__Memoize(#MakeICInaccessible)>>
function memo2() {
  return $x;
}

<<__Memoize(#SoftMakeICInaccessible)>>
function memo3() {
  return $x;
}

<<__EntryPoint>>
function main() {
  $x = HH\MemoizeOption#KeyedByIC;
  var_dump($x);
  var_dump(\__SystemLib\unwrap_opaque_value(\__SystemLib\OpaqueValueId::EnumClassLabel, $x));
  $x = new ReflectionFunction("memo1")->getAttributes()["__Memoize"][0];
  var_dump($x);
  var_dump(\__SystemLib\unwrap_opaque_value(\__SystemLib\OpaqueValueId::EnumClassLabel, $x));

  $x = HH\MemoizeOption#MakeICInaccessible;
  var_dump($x);
  var_dump(\__SystemLib\unwrap_opaque_value(\__SystemLib\OpaqueValueId::EnumClassLabel, $x));
  $x = new ReflectionFunction("memo2")->getAttributes()["__Memoize"][0];
  var_dump($x);
  var_dump(\__SystemLib\unwrap_opaque_value(\__SystemLib\OpaqueValueId::EnumClassLabel, $x));

  $x = HH\MemoizeOption#SoftMakeICInaccessible;
  var_dump($x);
  var_dump(\__SystemLib\unwrap_opaque_value(\__SystemLib\OpaqueValueId::EnumClassLabel, $x));
  $x = new ReflectionFunction("memo3")->getAttributes()["__Memoize"][0];
  var_dump($x);
  var_dump(\__SystemLib\unwrap_opaque_value(\__SystemLib\OpaqueValueId::EnumClassLabel, $x));
}
