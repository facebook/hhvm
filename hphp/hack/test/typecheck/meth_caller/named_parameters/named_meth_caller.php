<?hh
<<file: __EnableUnstableFeatures('named_parameters')>>

final class NamedCaller {
  public function required(int $positional, named int $x, named string $y): string {
    return $y;
  }

  public function optional(named int $x = 0): int {
    return $x;
  }

  public function positional_variadic(int ...$args, named string $tag): string {
    return $tag;
  }

  public function generic<T>(named T $value): T {
    return $value;
  }
}

function takes_caller((function(NamedCaller, int, named int $x, named string $y): string) $f): void {
  $f(new NamedCaller(), 1, y='ok', x=2);
}

function expect_int(int $_): void {}
function expect_string(string $_): void {}

function test_named_caller(): void {
  $obj = new NamedCaller();
  $required = HH\meth_caller(NamedCaller::class, 'required');
  takes_caller($required);
  $required($obj, 1, y='ok', x=2);
  expect_string($required($obj, 1, x=2, y='ok'));

  $optional = HH\meth_caller(NamedCaller::class, 'optional');
  $optional($obj);
  $optional($obj, x=3);

  $positional_variadic = HH\meth_caller(NamedCaller::class, 'positional_variadic');
  $positional_variadic($obj, tag='ok');
  $positional_variadic($obj, 1, 2, 3, tag='ok');

  $generic = HH\meth_caller(NamedCaller::class, 'generic');
  expect_int($generic($obj, value=3));
  expect_string($generic($obj, value='ok'));
}
