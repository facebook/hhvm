<?hh

class D {
}

function foo(inout int $x) :mixed{
  foreach (vec[new D] as $bar) {
    try {
      return $x;
    } finally {
      $x = $x + 1;
    }
  }
}

function main() :mixed{
  $y = 0;
  foo(inout $y);
  echo "done\n";
}


<<__EntryPoint>>
function main_inout_loop_2() :mixed{
main();
}
