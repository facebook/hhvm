<?hh

<<__EntryPoint>>
async function main() :Awaitable<mixed>{
  list($r, $w) = \HH\Lib\_Private\Native\pipe();
  await \HH\Asio\va(
    async {
      await HH\Asio\later();
      \fwrite($w, "OK\n");
    },
    async {
      await \stream_await($r, \STREAM_AWAIT_READ);
      print(\fgets($r));
    }
  );
}
