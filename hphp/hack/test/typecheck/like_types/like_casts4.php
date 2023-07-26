<?hh

class Cov<+T> {}
class Contra<-T> {}
class Inv<T> {}

function basics(dynamic $d): void {
  $d as ~Cov<_>;
  $d as ~Contra<_>;
  $d as ~Inv<_>;
}

function inner_types(dynamic $d): void {
  $d as ~shape('a' => vec<_>);

  $d as ~shape('a' => Vector<_>);
}
