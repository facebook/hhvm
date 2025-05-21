<?hh


class MemoSensitiveData implements HH\IPureMemoizeParam {
  public function getPayload()[]: int {
    return 42;
  }
  public function getInstanceKey()[]: string {
    return 'strkey';
  }
}


abstract final class IntContext extends HH\HHVMTestMemoSensitiveImplicitContext {
  const type TData = MemoSensitiveData;
  const ctx CRun = [zoned];
  public static function set<T>(this::TData $context, (function (): T) $f)[zoned, ctx $f]: T {
    return parent::runWith($context, $f);
  }
  public static function getContext()[zoned]: MemoSensitiveData {
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

<<__Memoize(#MakeICInaccessible)>>
function mici(): void {
  expect(HH\ImplicitContext\State::INACCESSIBLE);
}

<<__EntryPoint>>
function main(): void {
  expect(HH\ImplicitContext\State::INACCESSIBLE);
  IntContext::set(
    new MemoSensitiveData(0),
    () ==> expect(HH\ImplicitContext\State::VALUE),
  );
  expect(HH\ImplicitContext\State::INACCESSIBLE);
  mici();
}
