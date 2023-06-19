<?hh
<<file: __EnableUnstableFeatures('typed_local_variables')>>

function g<T1 as arraykey, T2>(T2 $b): dict<T1, vec<T2>> {
  return dict[];
}

function int_to_string(int $x): string {
  return "";
}

class C<T1 as int> {
  public function f<T>(int $a, T $b, T1 $c, mixed $y): void {
    $x = 4;
    let $x: int = 1;
    let $y: vec<mixed> = vec<int>[1];

    hh_expect_equivalent<int>($x);
    hh_expect_equivalent<vec<int>>($y);

    if ($a === 2) {
      let $t: T = $b;
      let $t1: T1 = $c;
      let $z: dict<T1, vec<T>> = g<T1, _>($b);
      hh_expect_equivalent<T>($t);
      hh_expect_equivalent<T1>($t1);
      hh_expect_equivalent<dict<T1, vec<T>>>($z);
    }
  }
  public function g(int $x): void {
    let $x: string = int_to_string($x);
    hh_expect_equivalent<string>($x);
    let $y: arraykey = 1;
    hh_expect_equivalent<int>($y);
    let $y: string = "s";
    hh_expect_equivalent<string>($y);
  }
}
