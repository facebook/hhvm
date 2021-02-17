<?hh

const vec<mixed> VALS = vec[
  0,
  -10,
  100,
  1.234,
  -3.4e10,
  INF,
  -INF,
  NAN,
  TRUE,
  FALSE,
  NULL,
  PHP_INT_MAX,
  STDIN,
  "string",
];

<<__EntryPoint>>
function main(): void {
  foreach(VALS as $i) {
    foreach(VALS as $j) {
      echo "<$i $j>\n";
    }
    "$i "; // this should generate a notice as applicable
    echo '' . $i;
    echo $i . '';
    echo "\n";
  }
}
