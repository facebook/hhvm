<?hh

class B {}
class C extends B {
  const type Te1 = shape(nameof self => float);
  const type Te2 = shape(self::class => float);

  const type T1 = shape(nameof C => float);
  const type T2 = shape(C::class => float);

  public function f(this::T1 $t1, this::T1 $t2): void {
    $t1[nameof static];
    $t1[static::class];
    $t2[nameof static];
    $t2[static::class];
    $t1[nameof parent];
    $t1[parent::class];
    $t2[nameof parent];
    $t2[parent::class];
  }
  public function g(this::T1 $t1, this::T1 $t2): void {
    Shapes::idx($t1, nameof static);
    Shapes::idx($t1, static::class);
    Shapes::idx($t2, nameof static);
    Shapes::idx($t2, static::class);
    Shapes::idx($t1, nameof parent);
    Shapes::idx($t1, parent::class);
    Shapes::idx($t2, nameof parent);
    Shapes::idx($t2, parent::class);
  }
}
