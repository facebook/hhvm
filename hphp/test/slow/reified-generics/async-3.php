<?hh

async function foo<reify T>($x): Awaitable<T> {
  return $x;
}

<<__EntryPoint>>
async function main() :Awaitable<mixed>{
  await foo<int>(1);
  echo "yep\n";
  await foo<string>(1);
}
