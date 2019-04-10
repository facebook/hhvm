<?hh

namespace JSCompat\Api {
  async function invoke(
    (function(): mixed) $thunk,
    mixed ...$args
  ): Awaitable<mixed>;
}
