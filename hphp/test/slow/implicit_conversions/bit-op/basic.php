<?hh

const vec<mixed> VALS = vec[
  0,
  -10,
  1.234,
  INF,
  NAN,
  true,
  false,
  null,
  STDIN,
  "string",
  varray[42],
  dict['foobar' => false],
];

<<__EntryPoint>>
function main(): void {
  foreach(VALS as $i) {
    if ($i is float || $i is int) {
      echo '$i '; var_dump($i);
      echo 'not<';
      ~$i; // this should generate a notice as applicable
      echo ~$i;
      echo ">\n";
    }
    foreach(VALS as $j) {
      echo '$i '; var_dump($i); echo '$j '; var_dump($j);
      do_the_thing($i, $j);
    }
  }
}

function do_the_thing(mixed $i, mixed $j): void {
  and($i, $j);
  or($i, $j);
  xor($i, $j);
  shl($i, $j);
  shr($i, $j);
}

function and(mixed $i, mixed $j): void {
  echo 'and<';
  $i & $j; // this should generate a notice as applicable
  $i &= $j;
  echo $i;
  echo ">\n";
}

function or(mixed $i, mixed $j): void {
  echo 'or<';
  $i | $j; // this should generate a notice as applicable
  $i |= $j;
  echo $i;
  echo ">\n";
}

function xor(mixed $i, mixed $j): void {
  echo 'xor<';
  $i ^ $j; // this should generate a notice as applicable
  $i ^= $j;
  echo $i;
  echo ">\n";
}

function shl(mixed $i, mixed $j): void {
  echo 'shl<';
  $i << $j; // this should generate a notice as applicable
  $i <<= $j;
  echo $i;
  echo ">\n";
}

function shr(mixed $i, mixed $j): void {
  echo 'shr<';
  $i >> $j; // this should generate a notice as applicable
  $i >>= $j;
  echo $i;
  echo ">\n";
}
