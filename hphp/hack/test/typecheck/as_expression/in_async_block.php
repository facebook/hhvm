<?hh // strict

async function f(mixed $x): Awaitable<void> {
  await async {
    expect_int($x as int);
  };
}

function expect_int(int $x): void {}
