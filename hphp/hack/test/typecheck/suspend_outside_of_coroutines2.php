<?hh // strict

async function f($b): Awaitable<int> {
  $a = suspend $b();
}
