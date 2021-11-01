<?hh

// Skipping an Awaitable hint's type parameters is an error.
async function foo(): Awaitable {}
