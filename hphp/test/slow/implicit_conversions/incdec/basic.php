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

<<__EntryPoint>>
function main(): void {
  foreach(VALS as $i) {
    var_dump($i);
    echo "preinc<";
    $l = $i;
    echo ++$l;
    echo "> postinc<";
    $l = $i;
    echo $l++;
    echo $l;
    echo "> predec<";
    $l = $i;
    echo --$l;
    echo "> postdec<";
    $l = $i;
    echo $l--;
    echo $l;
    echo ">\n";
  }
}
