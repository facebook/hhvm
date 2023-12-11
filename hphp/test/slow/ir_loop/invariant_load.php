<?hh

abstract final class AlternatorStatics {
  public static $i = 0;
}

function alternator() :mixed{
  mt_rand();
  mt_rand();
  mt_rand();
  return (AlternatorStatics::$i++ % 2) == 0;
}
function foo($x, $k) :mixed{
  foreach ($x as $j) {
    while(true) {
      echo "$j: ";
      echo $k;
      echo "\n";
      if (!alternator()) { break; }
    }
    echo "ok\n";
  }

  echo "done\n";
}


<<__EntryPoint>>
function main_invariant_load() :mixed{
foo(vec[1,2,3], 123);
foo(vec[1,2,3], 123);
foo(vec[1,2,3], 123);
foo(vec[1,2,3], 123);
foo(vec[1,2,3], 123);
foo(vec[1,2,3], 123);
foo(vec[1,2,3], 'asd');
}
