<?hh

final class ClassWithPureSleepWakeup {
  public function __sleep()[]: dict<nothing, nothing> {
    return dict[];
  }
  public function __wakeup()[]: void {}
}

final class ClassWithDefaultsSleepWakeup {
  public function __sleep()[defaults]: dict<nothing, nothing> {
    return dict[];
  }
  public function __wakeup()[defaults]: void {
  }
}

final class Ref {
  public function __construct(private mixed $inner) {}
}

function throws((function(): void) $fn): void {
  try {
    $fn();
  } catch (CoeffectViolationException $e) {
    return true;
  }
  return false;
}

const string TEST_KEY = 'test_apc_key';

function test_exception(string $test, mixed $value): void {
  $test_key = 'test_apc_key';
  echo "$test:\n";
  printf(
    "  throws on add: %s\n",
    throws(() ==> apc_add_with_pure_sleep($test_key, $value)) ? 'yes' : 'no',
  );
  printf(
    "  throws on store: %s\n",
    throws(() ==> apc_store_with_pure_sleep($test_key, $value)) ? 'yes' : 'no',
  );
  apc_store($test_key, $value);
  $success = false;
  printf(
    "  throws on fetch: %s\n",
    throws(() ==> apc_fetch_with_pure_wakeup($test_key, inout $success)) ? 'yes' : 'no',
  );
}

<<__EntryPoint>>
function main(): void {
  $obj_defaults = new ClassWithDefaultsSleepWakeup();
  test_exception('bare defaults object', $obj_defaults);
  test_exception('defaults object in vec', vec[$obj_defaults]);
  test_exception('defaults object in dict', dict['x' => $obj_defaults]);
  test_exception('defaults object in methodless object', new Ref($obj_defaults));

  $obj_pure = new ClassWithPureSleepWakeup();
  test_exception('bare pure object', $obj_pure);
  test_exception('pure object in vec', vec[$obj_pure]);
  test_exception('pure object in dict', dict['x' => $obj_pure]);
  test_exception('pure object in methodless object', new Ref($obj_pure));

  test_exception('int in methodless object', new Ref(3));
  test_exception('just arrays', vec[dict[], keyset[12, 3]]);
}
