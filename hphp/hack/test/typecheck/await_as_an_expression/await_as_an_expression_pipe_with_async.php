<?hh // strict

async function gen<T>(Awaitable<T> $gen): Awaitable<T> { return await $gen; }

async function foo(): Awaitable<int> {
  return await (
    async { return await async { return 42; }; }
    |> gen($$)
  );
}
