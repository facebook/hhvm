<?hh

async function gen_int(): Awaitable<int> {
  return 1;
}

function example(): Awaitable<mixed> {
  /*range-start*/
  echo "hello";
  $x = gen_int();
  /*range-end*/
}
