<?hh

async function foo<reify T>() {
  var_dump(HH\ReifiedGenerics\get_type_structure<T>());
}

<<__EntryPoint>>
async function main() {
  await foo<int>();
}
