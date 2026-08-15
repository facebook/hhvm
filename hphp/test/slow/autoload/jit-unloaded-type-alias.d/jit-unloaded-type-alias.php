<?hh

<<__NEVER_INLINE>>
function make_sync_result(): SyncResult {
  return shape('kind' => 'sync', 'value' => 1);
}

<<__NEVER_INLINE>>
function sync_result(): ?SyncResult {
  return make_sync_result();
}

<<__NEVER_INLINE>>
async function make_async_result(): Awaitable<AsyncResult> {
  await RescheduleWaitHandle::create(0, 0);
  return shape('kind' => 'async', 'value' => 2);
}

<<__NEVER_INLINE>>
async function async_result(): Awaitable<?AsyncResult> {
  return await make_async_result();
}

<<__NEVER_INLINE>>
function set_inout_result(inout InoutResult $result): void {
  $result = shape('kind' => 'inout', 'value' => 3);
}

<<__NEVER_INLINE>>
function inout_result(): ?InoutResult {
  $result = shape('kind' => 'initial', 'value' => 0);
  set_inout_result(inout $result);
  return $result;
}

<<__EntryPoint>>
async function main(): Awaitable<void> {
  var_dump(sync_result());
  var_dump(await async_result());
  var_dump(inout_result());
}
