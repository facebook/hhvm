<?hh // strict

function get_numbers(): Iterator<int> {
  yield 1;
  $a = coroutine function() {
    yield break;
  };
}
