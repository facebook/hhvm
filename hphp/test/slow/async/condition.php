<?hh

class Ref {
  function __construct(public $value)[] {}
}

async function noblock() :Awaitable<mixed>{
  echo "not blocking\n";
}

async function block() :Awaitable<mixed>{
  echo "block enter\n";
  await RescheduleWaitHandle::create(RescheduleWaitHandle::QUEUE_DEFAULT, 0);
  echo "block exit\n";
}

async function block_then_succeed(Ref $condition) :Awaitable<mixed>{
  echo "block enter\n";
  await RescheduleWaitHandle::create(RescheduleWaitHandle::QUEUE_DEFAULT, 0);
  echo "block exit\n";
  $condition->value->succeed(42);
}

async function block_then_fail(Ref $condition) :Awaitable<mixed>{
  echo "block enter\n";
  await RescheduleWaitHandle::create(RescheduleWaitHandle::QUEUE_DEFAULT, 0);
  echo "block exit\n";
  $condition->value->fail(new Exception('horrible failure'));
}
<<__DynamicallyCallable>>
async function condition_noblock() :Awaitable<mixed>{
  $condition = ConditionWaitHandle::create(noblock());
  echo "not reached\n";
}
<<__DynamicallyCallable>>
async function condition_block() :Awaitable<mixed>{
  $condition = ConditionWaitHandle::create(block());
  echo "constructed ConditionWaitHandle\n";
  await $condition;
  echo "not reached\n";
}
<<__DynamicallyCallable>>
async function condition_block_ugly_succeed() :Awaitable<mixed>{
  $condition = ConditionWaitHandle::create(block());
  echo "constructed ConditionWaitHandle\n";
  $condition->succeed(42);
  return await $condition;
}
<<__DynamicallyCallable>>
async function condition_block_ugly_fail() :Awaitable<mixed>{
  $condition = ConditionWaitHandle::create(block());
  echo "constructed ConditionWaitHandle\n";
  $condition->fail(new Exception('horrible failure'));
  return await $condition;
}
<<__DynamicallyCallable>>
async function condition_block_nice_succeed() :Awaitable<mixed>{
  $condition = new Ref(null);
  $condition->value = ConditionWaitHandle::create(
    block_then_succeed($condition)
  );
  echo "constructed ConditionWaitHandle\n";
  return await $condition->value;
}
<<__DynamicallyCallable>>
async function condition_block_nice_fail() :Awaitable<mixed>{
  $condition = new Ref(null);
  $condition->value = ConditionWaitHandle::create(block_then_fail($condition));
  echo "constructed ConditionWaitHandle\n";
  return await $condition->value;
}

async function run_one(string $name) :Awaitable<mixed>{
  try {
    echo "running $name...\n";
    $res = await $name();
    echo "succeeded: $res\n";
  } catch (Exception $e) {
    echo "failed: {$e->getMessage()}\n";
  }
  await RescheduleWaitHandle::create(RescheduleWaitHandle::QUEUE_DEFAULT, 1);
  echo "\n";
}

async function run() :Awaitable<mixed>{
  await run_one('condition_noblock');
  await run_one('condition_block');
  await run_one('condition_block_ugly_succeed');
  await run_one('condition_block_ugly_fail');
  await run_one('condition_block_nice_succeed');
  await run_one('condition_block_nice_fail');
}


<<__EntryPoint>>
function main_condition() :mixed{
HH\Asio\join(run());
}
