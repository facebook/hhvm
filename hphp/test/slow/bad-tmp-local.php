<?hh

function doit($v, $x) {
  $y = $x ?as int;
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
  var_dump(doit(vec[], "a"));
}
