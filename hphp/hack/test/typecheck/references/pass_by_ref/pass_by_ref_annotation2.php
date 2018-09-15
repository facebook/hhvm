<?hh

function foo(int $a, string& $b, bool $c): void {}

function test(): void {
  $i = 3;
  $s = 'x';
  $b = true;
  $foo_str = fun('foo');

  foo(42, &$s, true);
  foo(3, 'x', false);

  foo($i, $s, $b);
  foo($i, $s, &$b);
  foo(&$i, $s, $b);
  foo(&$i, $s, &$b);
  foo($i, &$s, $b);
  foo($i, &$s, &$b);
  foo(&$i, &$s, $b);
  foo(&$i, &$s, &$b);

  $foo_str(&$i, $s, $b);
  $foo_str($i, &$s, $b);
  $foo_str($i, $s, &$b);
}
