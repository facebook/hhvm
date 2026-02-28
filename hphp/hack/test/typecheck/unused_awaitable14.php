<?hh

async function f(): Awaitable<string> {
  return 'foobar';
}

async function g(): Awaitable<string> {
  $output = f();
  return '$foo'.(string)$output.'bar';
}
