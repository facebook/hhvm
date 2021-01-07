<?hh

type MyAwaitable = Awaitable<int>;

// Should error, because the Awaitable restriction is syntactic.
async function foo(): MyAwaitable { return 1; }
