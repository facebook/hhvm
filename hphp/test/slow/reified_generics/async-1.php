<?hh

async function foo<reify T>() {
  var_dump(HH\ReifiedGenerics\getType<T>());
}

<<__EntryPoint>>
async function main() {
  await foo<int>();
}
