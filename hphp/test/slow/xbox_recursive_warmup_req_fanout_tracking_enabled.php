<?hh

<<__DynamicallyCallable>>
function recursive_xbox(int $x) {
  var_dump($x);
  if ($x === 0) {
    return 1;
  }

  $awaitable = fb_gen_user_func_array(
    __FILE__,
    'recursive_xbox',
    vec[$x-1],
  );
  return HH\Asio\join($awaitable);
}

<<__EntryPoint>>
function main() {
  $ctx = HH\execution_context();
  echo "In $ctx\n";

  if (HH\execution_context() !== "xbox") {
    recursive_xbox(4);
  }

  // Wait for 1 second to allow the xbox resources to be cleaned up.
  // This is just to simplify matching line ordering of the expectf output,
  // which does not matter in real life.
  sleep(1);

  echo "$ctx done.\n";
}
