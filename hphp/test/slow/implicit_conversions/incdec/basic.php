<?hh

const vec<mixed> VALS = vec[
  null,
  false,
  true,
  0,
  42,
  1.234,
  'foobar',
  '',
  '1234',
  '1.234',
  STDIN,
];

class Foo {}

function with_exn(inout $x, $fn): void {
  try {
    echo $fn(inout $x);
  } catch (Exception $e) {
    echo "\n".$e->getMessage()."\n";
  }
}

<<__EntryPoint>>
function main(): void {
  foreach(VALS as $i) {
    var_dump($i);
    echo "preinc<";
    $l = $i;
    with_exn(inout $l, (inout $o) ==> ++$o);
    echo "> postinc<";
    $l = $i;
    with_exn(inout $l, (inout $o) ==> $o++);
    echo $l;
    echo "> predec<";
    $l = $i;
    with_exn(inout $l, (inout $o) ==> --$o);
    echo "> postdec<";
    $l = $i;
    with_exn(inout $l, (inout $o) ==> $o--);
    echo $l;
    echo ">\n";
  }
}
