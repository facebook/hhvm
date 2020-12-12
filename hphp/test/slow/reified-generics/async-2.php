<?hh

async function foo<reify T>($x) {
  var_dump($x is T);
}

<<__EntryPoint>>
async function main() {
  await foo<int>(1);
  await foo<string>(1);
}
