<?hh
<<file: __EnableUnstableFeatures('await_in_splice')>>

async function f(): Awaitable<void> {
  true ? <xhp x={BKS2`${await f()}`} /> : false;
}
