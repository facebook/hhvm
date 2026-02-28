<?hh

function foo(): (function(inout int): int) {
  return function(inout num $x) {
    return 42;
  };
}
