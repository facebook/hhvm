<?hh

// Coercion for concat/interp always throws; run() observes each case without
// aborting the rest of the test.
function run((function(): void) $f): void {
  try {
    $f();
  } catch (InvalidOperationException $e) {
    echo "[throw] " . $e->getMessage() . "\n";
  }
}

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
    run(() ==> { echo '' . $i; echo $i . ''; echo "\n"; });
    foreach($vals as $j) {
      run(() ==> { echo "<$i $j>\n"; });
      run(() ==> { $i_ = $i; var_dump($i_ .= $j); });
    }
  }
}
