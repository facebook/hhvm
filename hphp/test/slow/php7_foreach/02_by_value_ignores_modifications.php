<?hh

function run(inout $a) {
  foreach($a as $v) {
    echo "$v\n";
    unset($a[1]);
  }
}

<<__EntryPoint>>
function main() {
  $a = darray[0 => 1, 1 => 2, 2 => 3];
  run(inout $a);
}
