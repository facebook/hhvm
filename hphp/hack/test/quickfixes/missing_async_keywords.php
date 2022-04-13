<?hh

function foo(): Awaitable<void> {
  return await gen_bar();
}
