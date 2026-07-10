<?hh

// int argument used in interpolation converts without coercion (laundered + literal).
function run((function(): void) $f): void {
  try {
    $f();
  } catch (InvalidOperationException $e) {
    echo "[throw] " . $e->getMessage() . "\n";
  }
}

<<__EntryPoint>>
function main(): void {
  run(() ==> i(__hhvm_intrinsics\launder_value(42)));
  run(() ==> i(42));
}

function i(int $i): void { echo "int <$i>\n"; }
