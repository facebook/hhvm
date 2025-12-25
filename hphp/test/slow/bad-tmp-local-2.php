<?hh

function doit($v, $x) {
  try {
    $y = $x ?as string;
  } catch (Exception $e) {
    var_dump("caught");
  }
  var_dump(0);
  foreach ($v as $i) {
    var_dump(1);
    if ($i === 0) return 5;
  }
  var_dump(2);
  return 42;
}

<<__EntryPoint>>
function main() {
  set_error_handler(() ==> { throw new Exception; });
  var_dump(doit(vec[], stdclass::class));
}
