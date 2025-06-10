<?hh

class C { const type T = int; }

function f(classname<C> $cn, class<C> $c): void {
  type_structure($cn, 'T');
  type_structure($c, 'T');
}
