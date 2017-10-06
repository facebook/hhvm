<?hh

// Testing lambda
async function foo() {
  $nohint = async $x ==> await $x + 1;
  throw new Exception();
}
