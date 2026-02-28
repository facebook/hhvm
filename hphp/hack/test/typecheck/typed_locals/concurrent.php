<?hh
<<file: __EnableUnstableFeatures('typed_local_variables')>>

async function g(): Awaitable<int> {
  return 1;
}

async function f(): Awaitable<void> {
  concurrent {
    let $x:int = await g();
    $x = await g();
    let $y:int;
    $y = await g();
  }
  $x;
  $y;
  $x = 1;
}

async function f2(): Awaitable<void> {
  concurrent {
    let $x:int = await g();
    let $y:int;
  }
  $x;
  $y = 1;
}
