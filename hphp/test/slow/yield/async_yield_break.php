<?hh

async function fruit(): AsyncIterator<string> {
  echo "sadpanda, no fruit";
  yield break;
}


 <<__EntryPoint>>
async function main_async_yield_break(): Awaitable<void> {
  foreach (fruit() await as $fruit) {
    var_dump($fruit);
  }
}
