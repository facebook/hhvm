<?hh

async function gen(): Awaitable<vec<int>> {
  return vec[1, 2, 3];
}

function accept(int ...$args): void {
  foreach ($args as $arg) {
    echo $arg;
  }
}

<<__EntryPoint>>
async function main(): Awaitable<void> {
  accept(...await gen());
}

