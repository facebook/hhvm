<?hh

type X = resource;
newtype Y = ?resource;
newtype A = resource;
type B = ?resource;
newtype C = X;
type D = Y;
function a(<<__Soft>> A $x) :mixed{}
function b(<<__Soft>> B $x) :mixed{}
function c(<<__Soft>> C $x) :mixed{}
function d(<<__Soft>> D $x) :mixed{}

<<__EntryPoint>>
function main() :mixed{
  $x = imagecreate(10, 10);
  a(null);
  a(1);
  a($x);
  b(null);
  b(1);
  b($x);
  c(null);
  c(1);
  c($x);
  d(null);
  d(1);
  d($x);
  echo "Done\n";
}
