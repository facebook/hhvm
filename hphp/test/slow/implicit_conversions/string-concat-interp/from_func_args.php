<?hh

<<__EntryPoint>>
function main(): void {
  i(__hhvm_intrinsics\launder_value(42));
  b(__hhvm_intrinsics\launder_value(true));
  d(__hhvm_intrinsics\launder_value(1.234));
  n(__hhvm_intrinsics\launder_value(null));
  r(__hhvm_intrinsics\launder_value(HH\stdin()));
  m(__hhvm_intrinsics\launder_value(1.234));

  i(42);
  b(true);
  d(1.234);
  n(null);
  r(HH\stdin());
  m(1.234);
}

function i(int $i): void { echo "int <$i>\n"; }
function b(bool $i): void { echo "bool <$i>\n"; }
function d(float $i): void { echo "double <$i>\n"; }
function n(null $i): void { echo "null <$i>\n"; }
function r(resource $i): void { echo "resource <$i>\n"; }
function m(mixed $i): void { echo "mixed <$i>\n"; }
