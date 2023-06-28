<?hh

type X = callable;
newtype Y = ?callable;
newtype A = callable;
type B = ?callable;
newtype C = X;
type D = Y;
class Q {
  public static function foo() :mixed{}
}
function a(<<__Soft>> A $x) :mixed{}
function b(<<__Soft>> B $x) :mixed{}
function c(<<__Soft>> C $x) :mixed{}
function d(<<__Soft>> D $x) :mixed{}

<<__EntryPoint>>
function main() :mixed{
  a(null);
  a(1);
  a('var_dump');
  a('non_existent');
  a(varray['Q','foo']);
  a(varray['Q','non_existent']);
  a(function () { return 123; });
  b(null);
  b(1);
  b('var_dump');
  b('non_existent');
  b(varray['Q','foo']);
  b(varray['Q','non_existent']);
  b(function () { return 123; });
  c(null);
  c(1);
  c('var_dump');
  c('non_existent');
  c(varray['Q','foo']);
  c(varray['Q','non_existent']);
  c(function () { return 123; });
  d(null);
  d(1);
  d('var_dump');
  d('non_existent');
  d(varray['Q','foo']);
  d(varray['Q','non_existent']);
  d(function () { return 123; });
  echo "Done\n";
}
