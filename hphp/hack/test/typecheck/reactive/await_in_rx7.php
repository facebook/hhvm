<?hh // strict
<<file: __EnableUnstableFeatures('coeffects_provisional')>>

<<__Rx>>
async function f(): Awaitable<int> {
  return 1;
}

async function g(): Awaitable<void> {
  // OK
  $l = varray[f(), f()];
  $a = await gena($l);
}
