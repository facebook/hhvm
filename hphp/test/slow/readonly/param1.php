<?hh

function default_param(readonly int $v = 4)[]: void {
  $v = 5;
}

<<__EntryPoint>>
function main(): mixed {
  default_param();
  echo "done.\n";
}
