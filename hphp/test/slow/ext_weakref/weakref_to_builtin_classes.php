<?hh

// This tests that we invalidate WeakRefs to an instance when it is destroyed
// regardless of the instanceDtor it may have

function test($what) {
  echo "==== $what ====\n";
  $producer = 'produce_' . $what;
  $foo = $producer();
  $w = new WeakRef($foo);
  var_dump($w->valid());
  __hhvm_intrinsics\launder_value($foo);
  unset($foo);
  var_dump($w->valid());
}

function produce_stdClass() { return new stdClass(); }
function produce_native_data() { return new PDOStatement(); }
function produce_collection() { return Vector{}; }
async function produce_awaitable() { return 1; }
function produce_php_closure() { return function() { return 1; }; }
function produce_hack_closure() { return () ==> {}; }

<<__EntryPoint>> function main(): void {
  test('stdClass');
  test('native_data');
  test('collection');
  test('awaitable');
  test('php_closure');
  test('hack_closure');
}
