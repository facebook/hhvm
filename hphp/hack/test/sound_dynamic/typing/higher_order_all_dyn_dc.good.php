<?hh
// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

<<__DynamicallyCallable>>
function iter
  <Tv1 as supportdyn<mixed>>(
  vec<Tv1> $traversable,
  supportdyn<(function (Tv1): void)> $value_func,
): void {
}

<<__DynamicallyCallable>>
function print_line(string $_):void { }

function testit(vec<~string> $vs1, ~vec<string> $vs2):void {
  $f = print_line<>;
  // Works, because we infer Tv=string
  iter($vs2, $f);
  // Works, because we're explicit
  iter<string>($vs1, $f);
  // Does not work, because we commit to Tv=~string
  //iter($vs1, $f);
}
