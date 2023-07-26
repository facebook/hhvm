<?hh

class Cov<+T> {}
class Contra<-T> {}
class Inv<T> {}

function basics(dynamic $d): void {
  $d as ~Cov<int>;
  $d as ~Contra<int>;
  $d as ~Inv<int>;

  $d as ~shape('a' => int);
  $d as ~(int, int);

  $d as ~(function (int): string);
}

function inner_types(dynamic $d): void {
  $d as ~shape('a' => vec<int>);

  $d as ~shape('a' => Vector<int>);
}
