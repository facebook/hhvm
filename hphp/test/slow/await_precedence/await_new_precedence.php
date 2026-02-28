<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

async function f(): Awaitable<bool> {
  return true;
}

async function test(): Awaitable<void> {
  // currently an error because await f() is an expression to the ternary.
  $result = await f() ? true : false;
  var_dump($result);
}

<<__EntryPoint>>
function main() :mixed{
  \HH\Asio\join(test());
}
