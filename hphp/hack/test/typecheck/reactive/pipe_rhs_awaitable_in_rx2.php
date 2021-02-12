<?hh

async function toasync(int $a)[rx]: Awaitable<int> {
  return $a;
}


async function f()[rx]: Awaitable<void> {
  // ERROR
  1 |> toasync($$);
}
