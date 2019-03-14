<?hh // strict

async function foo(): Awaitable<int> {
  return await /* UNSAFE_EXPR */ async { return "string"; };
}
