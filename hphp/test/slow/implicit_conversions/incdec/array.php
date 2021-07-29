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

function with_exn(inout $x, $fn): void {
  try {
    echo $fn(inout $x);
  } catch (Exception $e) {
    echo "\n".$e->getMessage()."\n";
  }
}

<<__EntryPoint>>
function main(): void {
  $preinc  = VALS;
  $postinc = VALS;
  $predec  = VALS;
  $postdec = VALS;
  foreach (VALS as $i => $val) {
    var_dump($val);
    echo "preinc<";
    with_exn(inout $preinc[$i], (inout $o) ==> ++$o);
    echo "> postinc<";
    with_exn(inout $postinc[$i], (inout $o) ==> $o++);
    echo $postinc[$i];
    echo "> predec<";
    with_exn(inout $predec[$i], (inout $o) ==> --$o);
    echo "> postdec<";
    with_exn(inout $postdec[$i], (inout $o) ==> $o--);
    echo $postdec[$i];
    echo ">\n";
  }
}
