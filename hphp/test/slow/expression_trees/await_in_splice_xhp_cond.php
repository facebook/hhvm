<?hh

async function f(): Awaitable<void> {
  true ? <xhp x={BKS2`${await f()}`} /> : false;
}
