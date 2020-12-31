<?hh // strict
<<file: __EnableUnstableFeatures('coeffects_provisional')>>

<<__Rx>>
async function f()[rx]: Awaitable<int> {
  return 1;
}

async function g()[rx]: Awaitable<void> {
  // FIXME(coeffects) varray[...] should not require `defaults` context
  $l = varray[f(), f()];
  $a = await gena($l); // OK
}
