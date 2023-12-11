<?hh

function run(inout $a) :mixed{
  foreach($a as $v) {
    echo "$v\n";
    unset($a[1]);
  }
}

<<__EntryPoint>>
function main() :mixed{
  $a = dict[0 => 1, 1 => 2, 2 => 3];
  run(inout $a);
}
