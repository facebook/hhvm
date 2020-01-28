<?hh

function run(inout $a) {
  foreach($a as $v) {
    echo "$v\n";
    unset($a[1]);
  }
}

<<__EntryPoint>>
function main() {
  $a = varray[1, 2, 3];
  run(inout $a);
}
