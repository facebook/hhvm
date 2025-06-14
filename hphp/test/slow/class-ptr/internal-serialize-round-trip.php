<?hh

const int SER_INTERNAL = 6; // VariableSerializer::Type::Internal
const int DES_INTERNAL = 2; // VariableUnserializer::Type::Internal

class C {
  public static function f(): void { echo " called\n"; }
}

function round_trip($p): void {
  $key = $p[0];
  $val = $p[1];

  $x = __hhvm_intrinsics\serialize_with_format(
    $val,
    SER_INTERNAL,
  );
  $v = __hhvm_intrinsics\deserialize_with_format(
    $x,
    DES_INTERNAL,
  );
  echo "$key:";
  $v::f();
}

function pair_round_trip($p): void {
  $x = __hhvm_intrinsics\serialize_with_format(
    $p,
    SER_INTERNAL,
  );
  $pair = __hhvm_intrinsics\deserialize_with_format(
    $x,
    DES_INTERNAL,
  );
  $key = $pair[0];
  $val = $pair[1];

  echo "$key:";
  $val::f();
}

<<__EntryPoint>>
function main(): void {
  $ss = Pair { 'static_string', nameof C };
  $s = Pair { 'string', __hhvm_intrinsics\launder_value(nameof C)."" };
  $l = Pair { 'lazy_class', C::class };
  $c = Pair { 'class', HH\classname_to_class(C::class) };

  echo "===== Basic =====\n";
  round_trip($ss); // log
  round_trip($s); // log
  round_trip($l);
  round_trip($c);

  echo "\n\n===== Pair =====\n";
  pair_round_trip($ss); // log
  pair_round_trip($s); // log
  pair_round_trip($l);
  pair_round_trip($c);
}
