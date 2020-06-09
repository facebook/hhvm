<?hh

type X = num;
newtype Y = ?num;
newtype A = num;
type B = ?num;
newtype C = X;
type D = Y;
function a(<<__Soft>> A $x) {}
function b(<<__Soft>> B $x) {}
function c(<<__Soft>> C $x) {}
function d(<<__Soft>> D $x) {}

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
