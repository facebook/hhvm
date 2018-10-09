<?hh // strict

<<__Entrypoint>>
async function doStuff(): Awaitable<void> {
  $name = sys_get_temp_dir().'/'.bin2hex(random_bytes(8));
  $f = fopen($name, 'x+');
  await stream_await($f, STREAM_AWAIT_WRITE);
  \unlink($name);
  echo "Done.\n";
}
