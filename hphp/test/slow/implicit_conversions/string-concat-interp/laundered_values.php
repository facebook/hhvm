<?hh

// Like known.php, but operands are laundered so HHBBC/JIT can't constant-fold them.
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
    $i_ = __hhvm_intrinsics\launder_value($i);
    run(() ==> { echo '' . $i_; echo $i_ . ''; echo "\n"; });
    foreach($vals as $j) {
      $j_ = __hhvm_intrinsics\launder_value($j);
      run(() ==> { echo '<'.$i_.' '.$j_.">\n"; });
      run(() ==> { $i__ = $i_; var_dump($i__ .= $j_); });
    }
  }
}
