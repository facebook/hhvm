<?hh

async function f():Awaitable<dynamic> {
  return 0;
}

async function g():Awaitable<dynamic> {
  /* 'f()' is a partially typed expression of type Awaitable<_>,
   * and 'await f()' is an untyped expression. */
  return await f();
}
