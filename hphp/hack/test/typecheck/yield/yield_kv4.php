<?hh // strict

function f(): Generator<string, int, void> {
  yield 'one' => 1;
}

function expect<T>(T $x): void { }

function g(): void {
  foreach (f() as $k => $v) {
    expect<string>($k);
    expect<int>($v);
  }
}
