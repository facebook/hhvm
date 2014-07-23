<?hh // strict

function f(): Generator<string, int, void> {
  yield 'one' => 1;
}

function g(): void {
  foreach (f() as $k => $v) {
    hh_show($k);
    hh_show($v);
  }
}
