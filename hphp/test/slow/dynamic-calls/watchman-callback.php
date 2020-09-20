<?hh

<<__DynamicallyCallable>>
function callback(
  string $path,
  string $query,
  string $name,
  string $data_json,
  string $socket_path,
): void {
  require_once 'watchman_shared.inc';
  callback_impl($path, $query, $name, $data_json, $socket_path);
}

<<__EntryPoint>>
async function main(): Awaitable<void> {
  require_once 'watchman_shared.inc';
  await run_tests();
}
