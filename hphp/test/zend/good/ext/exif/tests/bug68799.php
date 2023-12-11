<?hh
/*
 * * Pollute the heap. Helps trigger bug. Sometimes not needed.
 * */
class A {
  function __construct() {
    $a = 'AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAa';
    $this->a = $a . $a . $a . $a . $a . $a;
  }
}

function doStuff ($limit) :mixed{

  $a = new A;

  $b = dict[];
  for ($i = 0; $i < $limit; $i++) {
    $b[$i] = clone $a;
  }

  unset($a);

  gc_collect_cycles();
}
<<__EntryPoint>> function main(): void {
$iterations = 3;

doStuff($iterations);
doStuff($iterations);

gc_collect_cycles();

print_r(exif_read_data(__DIR__.'/bug68799.jpg'));
}
