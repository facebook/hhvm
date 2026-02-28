<?hh

final class C {}

async function f(): Awaitable<classname<C>> { return C::class; }

async function g(): Awaitable<void> {
  // This is an error w/o the await, and probably should be an error with it
  new (await f())();
}
