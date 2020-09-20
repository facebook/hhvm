<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

class C<+T> {
  public function __construct(T $item) {}
}
class E<+Te> {}
class D<+Td super C<Td>> extends E<Td> {
  public function __construct(private Td $ditem) {}
  public function Get(): Td {
    return $this->ditem;
  }
}

function TestIt<Tt>(E<C<Tt>> $m): Tt {
  if ($m is D<_>) {
    // Should have $m : D<Td#1>
    // with C<Td#1> <: Td#1
    // D<Td#1> <: E<C<Tt>>
    // So Td#1 <: C<Tt>
    // But by transitivity we therefore have C<Td#1> <: C<Tt>
    // and so Td#1 <: Tt
    // So we can return $i as a Tt
    $i = $m->Get();
    return $i;
  }

  throw new Exception();
}
