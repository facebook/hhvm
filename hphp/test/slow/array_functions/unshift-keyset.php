<?hh

<<__EntryPoint>>
function main() :mixed{
  $x = keyset[0, 1];
  array_unshift(inout $x, 0);
  var_dump($x);

  $x = keyset[0, 1];
  array_unshift(inout $x, 1);
  var_dump($x);

  $x = keyset[4, 5];
  array_unshift(inout $x, 4);
  var_dump($x);

  $x = keyset[4, 5];
  array_unshift(inout $x, 5);
  var_dump($x);
}
