<?hh

// Coroutine blocks are not supported. This should result in a parsing error.

function test(): void {
  return coroutine {
    return 1;
  };
}
