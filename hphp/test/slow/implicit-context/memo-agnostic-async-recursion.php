<?hh

async function attr(): Awaitable<mixed>{
  $x = TestAsyncContext::getContext();
  if ($x > 10) return;
  var_dump($x);
  await TestAsyncContext::genRunWith($x+1, attr<>);
  var_dump(TestAsyncContext::getContext());
}

<<__EntryPoint>>
async function main(): Awaitable<mixed>{
  include 'memo-agnostic-async.inc';

  await TestAsyncContext::genRunWith(0, attr<>);
}
