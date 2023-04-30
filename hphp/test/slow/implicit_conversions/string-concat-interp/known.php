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
    "$i "; // this should generate a notice as applicable
    echo '' . $i;
    echo $i . '';
    echo "\n";
    foreach($vals as $j) {
      echo "<$i $j>\n";
      $i_ = $i;
      var_dump($i_ .= $j);
    }
  }
}
