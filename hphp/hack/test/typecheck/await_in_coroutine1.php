<?hh // strict

coroutine function f(Awaitable<string> $x) {
  $s = await $x;
}
