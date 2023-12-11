<?hh

function foo($x) :mixed{
  $r = vec[];
  foreach ($x as $v) { $r[] = $v; }
  return $r;
}

function main() :mixed{
  $heh = foo(vec[1,2,3]);
  foreach ($heh as $v) { var_dump($v); }
  echo "--\n";
  $heh = foo(vec[]);
  foreach ($heh as $v) { var_dump($v); }
}


<<__EntryPoint>>
function main_iter_006() :mixed{
main();
echo "done\n";
}
