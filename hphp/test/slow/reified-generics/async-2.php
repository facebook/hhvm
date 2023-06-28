<?hh

async function foo<reify T>($x) :Awaitable<mixed>{
  var_dump($x is T);
}

<<__EntryPoint>>
async function main() :Awaitable<mixed>{
  await foo<int>(1);
  await foo<string>(1);
}
