<?hh

<<__EntryPoint>>
function main() {
  $x = varray[0, 1];
  array_unshift(inout $x, 17);
  print("unshift(varray[0, 1], 17): current = ".current($x)."\n");
  $x = varray[4, 5];
  array_unshift(inout $x, 17);
  print("unshift(varray[4, 5], 17): current = ".current($x)."\n");

  $x = vec[0, 1];
  array_unshift(inout $x, 17);
  print("unshift(vec[0, 1], 17): current = ".current($x)."\n");
  $x = vec[4, 5];
  array_unshift(inout $x, 17);
  print("unshift(vec[4, 5], 17): current = ".current($x)."\n");

  $x = darray[0 => 0, 1 => 1];
  array_unshift(inout $x, 17);
  print("unshift(darray[0 => 0, 1 => 1], 17): current = ".current($x)."\n");
  $x = darray[4 => 4, 5 => 5];
  array_unshift(inout $x, 17);
  print("unshift(darray[4 => 4, 5 => 5], 17): current = ".current($x)."\n");

  $x = dict[0 => 0, 1 => 1];
  array_unshift(inout $x, 17);
  print("unshift(dict[0 => 0, 1 => 1], 17): current = ".current($x)."\n");
  $x = dict[4 => 4, 5 => 5];
  array_unshift(inout $x, 17);
  print("unshift(dict[4 => 4, 5 => 5], 17): current = ".current($x)."\n");

  $x = keyset[0, 1];
  array_unshift(inout $x, 17);
  print("unshift(keyset[0, 1], 17): current = ".current($x)."\n");
  $x = keyset[4, 5];
  array_unshift(inout $x, 17);
  print("unshift(keyset[4, 5], 17): current = ".current($x)."\n");
}
