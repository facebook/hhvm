<?hh

// Testing lambda
async function foo(): Awaitable<void> {
  $nohint = async $x ==> await $x + 1;
  throw new Exception();
}
