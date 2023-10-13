<?hh

class A {
  <<__Policied("P1")>>
  public int $i = 0;
}

trait T {
  <<__Policied("P2")>>
  public int $j = 0;
}

class B extends A {
  use T;

  <<__Policied("P3")>>
  public int $k = 0;
}

<<__InferFlows>>
function test(B $b): void {
  $b->i = $b->k;
  $b->j = $b->k;
}
