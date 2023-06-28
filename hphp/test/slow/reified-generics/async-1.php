<?hh

async function foo<reify T>() :Awaitable<mixed>{
  var_dump(HH\ReifiedGenerics\get_type_structure<T>());
}

<<__EntryPoint>>
async function main() :Awaitable<mixed>{
  await foo<int>();
}
