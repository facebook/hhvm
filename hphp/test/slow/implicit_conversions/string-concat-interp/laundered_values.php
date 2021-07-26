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
    $i_ = __hhvm_intrinsics\launder_value($i);
    "$i_ ";
    echo '' . $i_;
    echo $i_ . '';
    echo "\n";
    foreach(VALS as $j) {
      $j_ = __hhvm_intrinsics\launder_value($j);
      echo '<'.$i_.' '.$j_.">\n";
      $i__ = $i_;
      var_dump($i__ .= $j_);
    }
  }
}
