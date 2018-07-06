<?hh

function f(): int {
  function() use (): int {
    return 1;
  };
  return 1;
}
