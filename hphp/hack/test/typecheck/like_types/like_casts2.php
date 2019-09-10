<?hh

class Cov<+T> {}
class Contra<-T> {}
class Inv<T> {}

function basics(dynamic $d): void {
  $d as ~Cov<int>;
  $d as ~Contra<int>;
  $d as ~Inv<int>;

  // both types are covariant in their inner types
  $d as ~shape('a' => int);
  $d as ~(int, int);

  // error, parameter contravariant
  $d as ~(function (int): string);
}

function inner_types(dynamic $d): void {
  $d as ~shape('a' => vec<int>); // ok

  $d as ~shape('a' => Vector<int>); // error
}
