<?hh

// Only int/string operands remain (they concat/interpolate/.= without coercion);
// run() still surfaces any regressed coercion as [throw] rather than aborting.
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
    PHP_INT_MAX,
    "string",
  ];
  foreach($vals as $i) {
    run(() ==> { echo '' . $i; echo $i . ''; echo "\n"; });
    foreach($vals as $j) {
      run(() ==> { echo "<$i $j>\n"; });
      run(() ==> { $i_ = $i; $i_ .= $j; var_dump($i_); });
    }
  }
}
