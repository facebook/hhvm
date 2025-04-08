<?hh

namespace HH\SimpleLock {

function lock(string $name): Awaitable<void>;

function unlock(string $name): void;

function try_lock(string $name): bool;

function is_held(string $name): bool;

async function lock_with_timeout(string $name, int $timeout): Awaitable<void>;

}
