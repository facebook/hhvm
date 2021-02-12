<?hh

async function toasync(int $a)[rx]: Awaitable<int> {
  return $a;
}


async function f()[rx]: Awaitable<void> {
  // OK
  await (1 |> toasync($$));
}
