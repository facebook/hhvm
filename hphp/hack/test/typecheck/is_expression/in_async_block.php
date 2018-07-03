<?hh // strict

async function f(mixed $x): Awaitable<void> {
  await async {
    if ($x is int) {
      expect_int($x);
    }
  };
}

function expect_int(int $x): void {}
