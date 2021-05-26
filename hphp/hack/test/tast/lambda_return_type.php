<?hh

function f(): void {
  // The lambda should have a string return type
  (string $s) ==> $s;
}

async function g(): Awaitable<void> {
  // Async should have an Awaitable<int> type
  await async {
    return 42;
  };
}
