<?hh

type Ts1 = shape(C::class => float, 'C' => int);
type Ts2 = shape(nameof C => float, 'C' => int);

class B {
  const type Tb1 = shape(C::class => float, C::class => int);
  const type Tb2 = shape(C::class => float, nameof C => int);
  const type Tb3 = shape(nameof C => float, C::class => int);
  const type Tb4 = shape(nameof C => float, nameof C => int);
}
class C extends B {
  const type T1 = shape(B::class => float, C::class => int);
  const type T2 = shape(B::class => float, nameof C => int);
  const type T3 = shape(nameof B => float, C::class => int);
  const type T4 = shape(nameof B => float, nameof C => int);
}
