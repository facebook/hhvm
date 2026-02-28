<?hh
<<file: __EnableUnstableFeatures('expression_trees')>>

class C {
  public function f(int $s): bool {
    return false;
  }

  public function g(): bool {
    $l = $x ==> $x->f(1);
    return $l(new C());
  }
}

class D {
  public function map<T1, T2>((function(T1): T2) $f, vec<T1> $v): vec<T2> {
    $res = vec[];
    foreach ($v as $x) {
      $res[] = $f($x);
    }
    return $res;
  }

  public function test(): void {
    $this->map(($x ==> $x->f(1)), vec[new C()]);
  }

  public function test2(): vec<float> {
    $x = $this->map(
      ($x ==> $x + 1),
      vec[1.1, 2.1],
    ); // Needs constraint-based + or bidirectional
    $f = $x ==> $x + 1;
    $y = $this->map(
      $f,
      vec[1.1, 2.1],
    ); // Needs constraint-based, bidirectional doesn't help
    return $x;
  }

  public function test3(): void {
    $this->map(
      ($x ==> $x[0]),
      vec[vec[1], vec[2]],
    );
  }

  public function test4(dynamic $d): void {
    $x = $d->m(1, "2");
    $v = Vector<dynamic> {};
    $v[] = $x;
  }
}

function my_test1(): void {
  ExampleDsl`{
    $x = 1;
    $y = 2;
    return ${ExampleDsl`$x + $y`};
  }`;
}
