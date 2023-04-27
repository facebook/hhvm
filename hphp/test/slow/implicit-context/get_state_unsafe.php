<?hh

abstract final class IntContext extends HH\ImplicitContext {
  const type T = int;
  public static function set<T>(int $context, (function (): T) $f)[zoned, ctx $f]: T {
    return parent::runWith($context, $f);
  }
  public static function getContext()[zoned]: ?int {
    return parent::get();
  }
}

function expect(HH\ImplicitContext\State $expected): void {
  $actual = HH\ImplicitContext\get_state_unsafe();
  // since enum typehints aren't enforced, test it here
  $actual as HH\ImplicitContext\State;
  if ($actual === $expected) {
    echo "OK\n";
  } else {
    echo "Expected $expected, got $actual\n";
  }
}

<<__Memoize(#SoftMakeICInaccessible)>>
function soft_mici(): void {
  expect(HH\ImplicitContext\State::SOFT_INACCESSIBLE);
}

<<__Memoize(#MakeICInaccessible)>>
function mici(): void {
  expect(HH\ImplicitContext\State::INACCESSIBLE);
}

<<__EntryPoint>>
function main(): void {
  expect(HH\ImplicitContext\State::NULL);
  IntContext::set(
    0,
    () ==> expect(HH\ImplicitContext\State::VALUE),
  );
  HH\ImplicitContext\soft_run_with(
    () ==> expect(HH\ImplicitContext\State::SOFT_SET),
    '',
  );
  soft_mici();
  mici();
}
