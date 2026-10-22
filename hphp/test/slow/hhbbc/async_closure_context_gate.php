<?hh

async function gate_eager_leaf(): Awaitable<int> {
  return 13;
}

async function gate_apply(
  (function(): Awaitable<int>) $callback,
): Awaitable<int> {
  return await $callback();
}

async function gate_apply_if(
  bool $apply,
  (function(): Awaitable<int>) $callback,
): Awaitable<int> {
  if (!$apply) return 16;
  return await $callback();
}

async function gate_async_eager_closure(): Awaitable<int> {
  return await gate_apply(async () ==> 10);
}

async function gate_sync_closure(): Awaitable<int> {
  return await gate_apply(() ==> gate_eager_leaf());
}

async function gate_skipped_suspending_closure(): Awaitable<int> {
  return await gate_apply_if(false, async () ==> {
    await HH\Asio\later();
    return 17;
  });
}

<<__EntryPoint>>
function main(): void {
  var_dump(HH\Asio\join(gate_async_eager_closure()));
  var_dump(HH\Asio\join(gate_sync_closure()));
  var_dump(HH\Asio\join(gate_skipped_suspending_closure()));
}
