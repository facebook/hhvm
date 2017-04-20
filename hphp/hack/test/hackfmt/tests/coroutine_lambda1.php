<?hh

function f(): (function(int): int) {
  return coroutine $v ==> 1;
}
