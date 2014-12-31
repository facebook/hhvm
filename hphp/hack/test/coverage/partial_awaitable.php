<?hh

async function f() {
  // UNSAFE
}

async function g() {
  /* 'f()' is a partially typed expression of type Awaitable<_>,
   * and 'await f()' is an untyped expression. */
  return await f();
}
