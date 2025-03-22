<?hh

class X {}

class C {
  public static function m<Te>(): void { echo "C::m\n"; }
  public static function r<reify Tr>(): void { var_dump(HH\ReifiedGenerics\get_classname<Tr>()); }
  const string S = "hi";
}

class R<reify T> {
  public static function m<Te>(): void { echo "R::m\n"; }
  public static function r<reify Tr>(): void { var_dump(HH\ReifiedGenerics\get_classname<Tr>()); }
  const string S = "hi";
}


function f<reify Treify, Terase>(): void {
  echo ">> T::cm\n";
  Treify::m();
  Treify::m<Terase>();
  Treify::m<X>();
  Treify::m<Tr>();

  // TODO(T218390557)
  // echo ">> T::r\n";
  // Treify::r<X>();
  // Treify::r<Tr>();

  echo ">> T::TS\n";
  var_dump(Treify::S);
  echo ">> T::cm<>\n";
  $m1 = Treify::m<>;
  $m2 = Treify::m<Terase>;
  $m3 = Treify::m<X>;
  $m4 = Treify::m<Treify>;
  $m1();
  $m2();
  $m3();
  $m4();
  echo ">> T::cr<>\n";
  $r1 = Treify::r<X>;
  $r1();
  // TODO(T218390557)
  // $r2 = Treify::r<Treify>;
  // $r2();
}

<<__EntryPoint>>
function main(): void {
  echo "=== C ===\n";
  f<C, X>();

  echo "\n\n=== R<int> ===\n";
  f<R<X>, X>();
}
