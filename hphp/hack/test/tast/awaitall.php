<?hh // strict

async function genInt(): Awaitable<int> {
  return 1;
}

async function genString(): Awaitable<string> {
  return "foo";
}

async function f(): Awaitable<void> {
  concurrent {
    $a = await genInt();
    await genString();
    $c = await genString();
  }
}
