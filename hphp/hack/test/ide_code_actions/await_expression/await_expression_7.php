<?hh

async function gen_int(): Awaitable<int> {
  return 1;
}

function example(): Awaitable<mixed> {
  return () ==> {
    // `asnyc` should be added to this lambda, not the one above
    return () ==> {
      $_ = /*range-start*/gen_int()/*range-end*/ + 3;
    }
  }
}
