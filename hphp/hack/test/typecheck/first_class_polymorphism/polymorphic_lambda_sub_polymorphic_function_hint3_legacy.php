<?hh

<<file: __EnableUnstableFeatures('polymorphic_lambda','polymorphic_function_hints')>>

function expect<T>(T $_): void {}

function thing(): void {
  $f = function<T1>(T1 $x): (function<T2>(T2): (T1,T2)) use() {
    return function<T2>(T2 $y): (T1,T2) use($x) { return tuple($x, $y); };
  };

  $g = $f(1);
  $h = $g('a');
  expect<(int,string)>($h);
}
