<?hh

<<__AsioLowPri>>
async function asio_test1(): Awaitable<void> {
  // allowed
}

<<__AsioLowPri>>
async function asio_test2(): AsyncGenerator<int, string, void> {
  // not allowed (must be non-generator async function)
  yield 42 => 'str';
}

<<__AsioLowPri>>
function asio_test3(): void {
  // not allowed (must be non-generator async function)
}
