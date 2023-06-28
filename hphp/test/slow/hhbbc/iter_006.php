<?hh

function foo($x) :mixed{
  $r = varray[];
  foreach ($x as $v) { $r[] = $v; }
  return $r;
}

function main() :mixed{
  $heh = foo(varray[1,2,3]);
  foreach ($heh as $v) { var_dump($v); }
  echo "--\n";
  $heh = foo(varray[]);
  foreach ($heh as $v) { var_dump($v); }
}


<<__EntryPoint>>
function main_iter_006() :mixed{
main();
echo "done\n";
}
