<?hh // strict

function dv(): dict<string, vec<int>> {
  return dict['foo' => vec[2, 4, 6]];
}

function f(inout arraykey $x): void {
  $x = 'bar';
}

function test(): dict<string, vec<int>> {
  $dv = dv();
  f(inout $dv['foo'][2]);
  return $dv;
}
