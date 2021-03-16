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

<<__EntryPoint>>
function main(): void {
  $preinc  = VALS;
  $postinc = VALS;
  $predec  = VALS;
  $postdec = VALS;
  foreach (VALS as $i => $val) {
    var_dump($val);
    echo "preinc<";
    echo ++$preinc[$i];
    echo "> postinc<";
    echo $postinc[$i]++;
    echo $postinc[$i];
    echo "> predec<";
    echo --$predec[$i];
    echo "> postdec<";
    echo $postdec[$i]--;
    echo $postdec[$i];
    echo ">\n";
  }
}
