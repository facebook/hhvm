<?hh // strict

async function test(): Awaitable<void> {
  if (await async { return true; }) {
    var_dump(await async { return 0; });
  }
  if (await async {}) {
    if (await async {}) {
    }
  }
}

<<__EntryPoint>>
function main() {
  \HH\Asio\join(test());
}
