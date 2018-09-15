<?hh // strict

async function f($b, $c): AsyncIterator<int> {
  $r = await $b;
  yield 10;
  $a = suspend $c();
}
