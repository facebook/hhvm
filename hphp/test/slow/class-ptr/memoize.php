<?hh

<<file:__EnableUnstableFeatures('class_type')>>

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

<<__Memoize>>
function x(class<C> $c):     mixed { echo "** Called x\n"; return $c; }
<<__Memoize>>
function y(class<C> $c):     mixed { echo "** Called y\n"; return $c; }
<<__Memoize>>
function z(class<C> $c):     mixed { echo "** Called z\n"; return $c; }

<<__Memoize>>
function ax(class_or_classname<C> $c):     mixed { echo "** Called x\n"; return $c; }
<<__Memoize>>
function by(class_or_classname<C> $c):     mixed { echo "** Called y\n"; return $c; }
<<__Memoize>>
function cz(class_or_classname<C> $c):     mixed { echo "** Called z\n"; return $c; }

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

  echo "===== class<C> =====\n";
  p(x($s));
  p(x($s));
  p(x($l));
  p(x($c));

  p(y($l));
  p(y($s));
  p(y($l));
  p(y($c));

  p(z($c));
  p(z($s));
  p(z($l));
  p(z($c));

  echo "===== class_or_classname<C> =====\n";
  p(ax($s));
  p(ax($s));
  p(ax($l));
  p(ax($c));

  p(by($l));
  p(by($s));
  p(by($l));
  p(by($c));

  p(cz($c));
  p(cz($s));
  p(cz($l));
  p(cz($c));
}
