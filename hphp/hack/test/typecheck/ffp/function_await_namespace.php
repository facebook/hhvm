<?hh

namespace Foo {
  class Awaitable<T> {}

  // Awaitable is autoloaded, so the following are equivalent and do
  // not refer to the class in this namespace.
  async function bar(): Awaitable<int> { return 1; }
  async function baz(): \HH\Awaitable<int> { return 1; }
}
