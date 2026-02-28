<?hh

class D {}
class C {
  const type T = D;
}
function f(class<D> $c): void {
  $d = type_structure(C::class, 'T')['classname'];

  f($d);
}
