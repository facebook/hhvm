<?hh

function expect_float(float $f): void {}
class C {
  const type T1 = shape(nameof C => float);
  const type T2 = shape(C::class => float);

  public function f(this::T1 $t1, this::T1 $t2): void {
    expect_float($t1[nameof self]);
    expect_float($t1[self::class]);
    expect_float($t2[nameof self]);
    expect_float($t2[self::class]);
  }
}
