<?hh

async function foo() {
  $a = 123;
  $x = await $a();
  var_dump($x);
}

<<__EntryPoint>>
function main(): void {
  foo();
}
