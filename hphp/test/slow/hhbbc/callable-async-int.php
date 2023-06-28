<?hh

async function foo() :Awaitable<mixed>{
  $a = 123;
  $x = await $a();
  var_dump($x);
}

<<__EntryPoint>>
function main(): void {
  foo();
}
