<?hh // strict

async function asyncFunction() {
  $a = coroutine function(Awaitable<string> $x) {
    $s = await $x;
  };
}
