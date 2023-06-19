<?hh
<<file: __EnableUnstableFeatures('typed_local_variables')>>

class C<T1 as int> {
  public function f<T>(int $a, T $b, T1 $c): void {
    let $x: int = $b;
    let $y: T1 = $b;
    let $z: T = $c;
    let $w: vec<int> = vec[$b];
    let $v: nothing = 1;
  }
}
