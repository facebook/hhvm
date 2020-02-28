<?hh

function foo($x) {
  $r = varray[];
  foreach ($x as $v) { $r[] = $v; }
  return $r;
}

function main() {
  $heh = foo(varray[1,2,3]);
  foreach ($heh as $v) { var_dump($v); }
  echo "--\n";
  $heh = foo(varray[]);
  foreach ($heh as $v) { var_dump($v); }
}


<<__EntryPoint>>
function main_iter_006() {
main();
echo "done\n";
}
