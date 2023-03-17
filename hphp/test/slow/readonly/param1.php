<?hh

function foo()[]: int { return 4; }

function default_param(readonly int $v = readonly(foo()))[]: void {
  $v = 5;
}

<<__EntryPoint>>
function main() {
  default_param();
  echo "done.\n";
}
