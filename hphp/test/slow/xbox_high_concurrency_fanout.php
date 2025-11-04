<?hh

<<__DynamicallyCallable>>
function foo(int $i) {
  var_dump($i);
}



<<__EntryPoint>>
function main() {
  if (HH\execution_context() === "xbox") {
    return;
  }

  $iterations = vec(range(0, 10000));
  $awaitables = vec[];

  foreach ($iterations as $i) {
    $awaitables[] = fb_gen_user_func_array(
      __FILE__,
      'foo',
      vec[$i],
    );
  }

  $combined = AwaitAllWaitHandle::fromVec($awaitables);

  HH\Asio\join($combined);
}
