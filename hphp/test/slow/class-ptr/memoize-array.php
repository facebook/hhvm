<?hh

class C {}

<<__Memoize>>
function a(dict<int, classname<C>> $c): mixed { echo "** Called a\n"; return $c; }
<<__Memoize>>
function b(dict<int, classname<C>> $c): mixed { echo "** Called b\n"; return $c; }
<<__Memoize>>
function c(dict<int, classname<C>> $c): mixed { echo "** Called c\n"; return $c; }

<<__EntryPoint>>
function main(): void {
  $ds = dict[1 => nameof C];
  $l = C::class;
  $dl = dict[1 => $l];
  $dc = dict[1 => HH\classname_to_class($l)];

  a($ds);
  a($ds);
  a($dl);
  a($dc);

  b($dl);
  b($ds);
  b($dl);
  b($dc);

  c($dc);
  c($ds);
  c($dl);
  c($dc);
}
