<?hh

async function foo() :AsyncGenerator<mixed,mixed,void>{
  yield 42;
  echo "waiting for clearing ref\n";
  await RescheduleWaitHandle::create(0, 0);
  echo "finishing and destructing\n";
}

<<__EntryPoint>>
async function main() :Awaitable<mixed>{
  await RescheduleWaitHandle::create(0, 0);
  $gen = foo();
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
