<?hh

newtype Foo = int;
newtype Foo2 = int;

function foo(Foo $x, Foo2 $y) {}

newtype Bar = float;

function bar(Bar $k) {}

class bazcls {}
newtype Baz = BazCls;
function baz(Baz $k) {}
function baz2(Bazcls $k) {}

type A = varray;
type B = int;
type C = int;
type D = bool;
type E = bool;
type F = string;
type G = float;
type H = float;
type I = float;
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

type A_ = a;
type B_ = b;
type C_ = c;
type D_ = d;
type E_ = e;
type F_ = f;
type G_ = g;
type H_ = h;
type I_ = i;

function lots2(A_ $x,
               B_ $xx,
               C_ $xxx,
               D_ $xxxx,
               E_ $xxxxx,
               F_ $xxxxxx,
               G_ $xxxxxxx,
               H_ $xxxxxxxx,
               I_ $xxxxxxxxx)
{}

function lots3(a_ $x,
               b_ $xx,
               c_ $xxx,
               d_ $xxxx,
               e_ $xxxxx,
               f_ $xxxxxx,
               g_ $xxxxxxx,
               h_ $xxxxxxxx,
               i_ $xxxxxxxxx)
{}

function main() {
  foo(12, 13);
  bar(1.0);
  baz(new Bazcls());
  baz2(new Bazcls());
  lots(varray[],
       12,
       12,
       false,
       true,
       "asd",
       1.3,
       1.4,
       1.5);
  lots2(varray[],
        12,
        12,
        false,
        true,
        "asd",
        1.3,
        1.4,
        1.5);
  lots3(varray[],
        12,
        12,
        false,
        true,
        "asd",
        1.3,
        1.4,
        1.5);
}


<<__EntryPoint>>
function main_typedef_mixed_case() {
main();
}
