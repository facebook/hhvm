<?hh

type X = arraykey;
newtype Y = ?arraykey;
newtype A = arraykey;
type B = ?arraykey;
newtype C = X;
type D = Y;
function a(@A $x) {}
function b(@B $x) {}
function c(@C $x) {}
function d(@D $x) {}

<<__EntryPoint>>
function main() {
  a(null);
  a(1);
  a(1.0);
  a('foo');
  b(null);
  b(1);
  b(1.0);
  b('foo');
  c(null);
  c(1);
  c(1.0);
  c('foo');
  d(null);
  d(1);
  d(1.0);
  d('foo');
  echo "Done\n";
}
