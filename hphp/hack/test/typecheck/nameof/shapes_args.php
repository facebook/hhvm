<?hh

class C {}
type T1 = shape(nameof C => float);
type T2 = shape(C::class => float);

function f(T1 $t1, T2 $t2): void {
  f($t1, $t1);
  f($t1, $t2);
  f($t2, $t1);
  f($t2, $t2);
}
