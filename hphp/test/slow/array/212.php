<?hh


<<__EntryPoint>>
function main_212() :mixed{
  $a = varray[1, 2];
  $b = $a;
  $a[] = 3;
  var_dump($b);
}
