<?hh

// Regression test for T267373869.
//
// A crafted serialized string with an aggregate value (collection, array, or
// object) in key position (an array key, a collection key, or an object
// property name) used to trigger a heap use-after-free: the aggregate
// registered pointers into its own interior storage as back-reference (r:N)
// targets, but that storage was freed as soon as the temporary key was
// destroyed, leaving a later r:N resolving to freed memory. Aggregates are
// never produced by serialize() in key position, so they are now rejected
// during parsing.

function attempt(string $payload): void {
  // Each of these must be cleanly rejected (false), never crash.
  var_dump(unserialize($payload) === false);
}

<<__EntryPoint>>
function main(): void {
  // Swallow the "Unable to unserialize" notices so the rejected payloads
  // produce deterministic output (just the bool(true) from attempt()).
  set_error_handler((int $_errno, string $_msg) ==> true);

  // Original PoC: a Map ("StableMap") as a property name of an stdClass,
  // followed by a back-reference (r:3) into the freed Map's interior.
  attempt('O:8:"stdClass":2:{K:9:"StableMap":1:{i:1;s:3:"one";}s:1:"v";s:1:"x";r:3;}');
  // Dict in property-name position.
  attempt('O:8:"stdClass":1:{D:1:{s:1:"a";i:1;}s:1:"v";}');
  // Vec in array-key position.
  attempt('a:1:{v:1:{i:0;}i:1;}');
  // Map in collection-key position.
  attempt('K:9:"StableMap":1:{K:9:"StableMap":1:{i:1;i:2;}i:5;}');

  echo "--- valid inputs round-trip unchanged ---\n";

  // A Map used as a *value* (legitimate) still unserializes.
  $m = Map {1 => 'one'};
  var_dump(unserialize(serialize($m)) == $m);

  // A shared object forces serialize() to emit a back-reference (r:N); the
  // restored values must remain the same object.
  $shared = new stdClass();
  $shared->v = 1;
  $container = new stdClass();
  $container->x = $shared;
  $container->y = $shared;
  $restored = unserialize(serialize($container));
  var_dump($restored->x === $restored->y);

  // Plain string/int-keyed structures still unserialize.
  var_dump(unserialize('a:2:{i:0;s:5:"hello";s:3:"key";i:7;}'));
}
