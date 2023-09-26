<?hh
<<file: __EnableUnstableFeatures('typed_local_variables')>>

async function g(): Awaitable<int> {
  return 1;
}

async function h(): Awaitable<string> {
  return "";
}

async function f(): Awaitable<void> {
  $b = 1;
  concurrent {
    let $x:int = await h(); // error
    $y = await g();
    let $z:int;
    $z = await h(); // error
    let $y:int; // error
    $a = await g();
    let $a:int; // error
    let $b:int; // error
  }
}


async function f2(): Awaitable<void> {
  concurrent {
    let $a:int = 1;
    $a = await h();  // error, we require $a : int
  }
}
