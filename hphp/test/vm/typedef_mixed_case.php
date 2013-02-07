<?hh

newtype Foo = Int;
newtype Foo2 = Integer;

function foo(Foo $x, Foo2 $y) {}

newtype Bar = floAt;

function bar(Bar $k) {}

class bazcls {}
newtype Baz = BazCls;
function baz(Baz $k) {}
function baz2(Bazcls $k) {}

type A = aRray;
type B = iNt;
type C = iNteger;
type D = bOol;
type E = bOolean;
type F = sTring;
type G = rEal;
type H = fLoat;
type I = dOuble;
function lots(A $x,
              B $xx,
              C $xxx,
              D $xxxx,
              E $xxxxx,
              F $xxxxxx,
              G $xxxxxxx,
              H $xxxxxxxx,
              I $xxxxxxxxx)
{}

function main() {
  foo(12, 13);
  bar(1.0);
  baz(new Bazcls());
  baz2(new Bazcls());
  lots(array(),
       12,
       12,
       false,
       true,
       "asd",
       1.3,
       1.4,
       1.5);
}

main();