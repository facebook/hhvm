<?hh

function unknown($x) :mixed{
  return \HH\global_get('asd');
}

function foo($ids) :mixed{
  $x = dict[];
  foreach ($ids as $id) {
    $target = unknown($id);
    if ($target !== null) {
      if (!array_key_exists($target, $x)) $x[$target] = vec[];
      $x[$target][] = $id;
    }
  }
  return $x;
}
function main() :mixed{
  $x = foo(vec[1,2,3]);
  foreach ($x as $k => $v) {
    var_dump($v);
  }
}
<<__EntryPoint>>
function entrypoint_array_057(): void {

  \HH\global_set('asd', '2'.mt_rand());

  main();
}
