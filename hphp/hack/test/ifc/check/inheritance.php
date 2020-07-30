<?hh // strict

class A {
  <<Policied("P1")>>
  public int $i = 0;
}

trait T {
  <<Policied("P2")>>
  public int $j = 0;
}

class B extends A {
  use T;

  <<Policied("P3")>>
  public int $k = 0;
}

function test(B $b): void {
  $b->i = $b->k;
  $b->j = $b->k;
}
