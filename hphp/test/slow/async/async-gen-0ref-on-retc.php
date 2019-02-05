<?hh // decl

class Foo {
}

async function foo() {
  yield 42;
  echo "waiting for clearing ref\n";
  await RescheduleWaitHandle::create(0, 0);
  echo "finishing and destructing\n";
}

async function main() {
  await RescheduleWaitHandle::create(0, 0);
  $gen = foo();
  $gen->destructGuard = new Foo();
  $next = await $gen->next();
  var_dump($next[1]);
  echo "iterating\n";
  $gen->next();
  echo "clearing ref\n";
  $gen = null;
  echo "waiting for foo to finish\n";
  await RescheduleWaitHandle::create(0, 1);
  echo "survived\n";
}


<<__EntryPoint>>
function main_async_gen_0ref_on_retc() {
\HH\Asio\join(main());
}
