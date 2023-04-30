<?hh


<<__EntryPoint>>
function main(): void {
  $vals = vec[
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
    HH\stdin(),
    "string",
  ];
  foreach($vals as $i) {
    $i_ = __hhvm_intrinsics\launder_value($i);
    "$i_ ";
    echo '' . $i_;
    echo $i_ . '';
    echo "\n";
    foreach($vals as $j) {
      $j_ = __hhvm_intrinsics\launder_value($j);
      echo '<'.$i_.' '.$j_.">\n";
      $i__ = $i_;
      var_dump($i__ .= $j_);
    }
  }
}
