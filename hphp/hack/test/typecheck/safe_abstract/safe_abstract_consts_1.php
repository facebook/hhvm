<?hh

abstract class C1 {
  abstract const int abs;
  const int concr = 1;
}

final abstract class C2 extends C1 {
  const int abs = 1;
}

function foo(): void {
  $class = C1::class;
  $class::abs; // warn
  $class::concr; // OK
}
