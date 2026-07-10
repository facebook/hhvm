<?hh

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
  run(() ==> b(__hhvm_intrinsics\launder_value(true)));
  run(() ==> d(__hhvm_intrinsics\launder_value(1.234)));
  run(() ==> n(__hhvm_intrinsics\launder_value(null)));
  run(() ==> r(__hhvm_intrinsics\launder_value(HH\stdin())));
  run(() ==> m(__hhvm_intrinsics\launder_value(1.234)));

  run(() ==> i(42));
  run(() ==> b(true));
  run(() ==> d(1.234));
  run(() ==> n(null));
  run(() ==> r(HH\stdin()));
  run(() ==> m(1.234));
}

function i(int $i): void { echo "int <$i>\n"; }
function b(bool $i): void { echo "bool <$i>\n"; }
function d(float $i): void { echo "double <$i>\n"; }
function n(null $i): void { echo "null <$i>\n"; }
function r(resource $i): void { echo "resource <$i>\n"; }
function m(mixed $i): void { echo "mixed <$i>\n"; }
