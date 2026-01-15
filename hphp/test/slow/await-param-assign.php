<?hh

<<file:__EnableUnstableFeatures('allow_extended_await_syntax', 'allow_conditional_await_syntax')>>

async function bar() {
  return 1;
}

async function foo(int $i = await bar()) {
  var_dump($i);
}

<<__EntryPoint>>
async function main() {
  await foo();
  await foo(1);
}
