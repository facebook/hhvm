<?hh

type X = num;
newtype Y = ?num;
newtype A = num;
type B = ?num;
newtype C = X;
type D = Y;
function a(<<__Soft>> A $x) :mixed{}
function b(<<__Soft>> B $x) :mixed{}
function c(<<__Soft>> C $x) :mixed{}
function d(<<__Soft>> D $x) :mixed{}

<<__EntryPoint>>
function main() :mixed{
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
