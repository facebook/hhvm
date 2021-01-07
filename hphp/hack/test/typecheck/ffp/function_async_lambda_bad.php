<?hh

// Explicit return types on lambdas must be Awaitable.

function foo(): void {
  $f = async (): int ==> 1;
}

function bar(): void {
  $f = async function(): int { return 1; };
}
