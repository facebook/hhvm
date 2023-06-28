<?hh
function s() :mixed{
  $storage = varray[varray['x', 'y']];
  return $storage[0];
}
<<__EntryPoint>> function main(): void {
foreach (s() as $k => $function) {
  echo "op1 $k\n";
  if ($k == 0) {
    foreach (s() as $k => $function) {
      echo "op2 $k\n";
    }
  }
}
}
