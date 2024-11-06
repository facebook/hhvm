<?hh

class C {}

function p($c): void {
  __hhvm_intrinsics\debug_var_dump_lazy_class($c);
}

<<__Memoize>>
function a(classname<C> $c): mixed { echo "** Called a\n"; return $c; }
<<__Memoize>>
function b(classname<C> $c): mixed { echo "** Called b\n"; return $c; }
<<__Memoize>>
function c(classname<C> $c): mixed { echo "** Called c\n"; return $c; }

<<__EntryPoint>>
function main(): void {
  $s = nameof C;
  $l = C::class;
  $c = HH\classname_to_class($l);

  echo "===== classname<C> =====\n";
  p(a($s));
  p(a($s));
  p(a($l));
  p(a($c));

  p(b($l));
  p(b($s));
  p(b($l));
  p(b($c));

  p(c($c));
  p(c($s));
  p(c($l));
  p(c($c));
}
