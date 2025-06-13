<?hh

class C {
  public static function f(): void { echo " called\n"; }
}

function round_trip($p): void {
  $key = $p[0];
  $val = $p[1];

  $x = serialize($val);
  $v = unserialize($x);
  echo "$key:";
  $v::f();
}

function pair_round_trip($p): void {
  $x = serialize($p);
  $pair = unserialize($x);
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
