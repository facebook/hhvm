<?hh

interface I {}

function foo(
  I $i // TODO(T140068390): should not infer a shape
): void {}

async function gen_void():
    Awaitable<void> {} // TODO(T140068390): should not infer a shape
