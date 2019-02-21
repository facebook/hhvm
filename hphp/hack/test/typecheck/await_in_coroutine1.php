<?hh // strict

coroutine function f(Awaitable<string> $x): void {
  $s = await $x;
}
