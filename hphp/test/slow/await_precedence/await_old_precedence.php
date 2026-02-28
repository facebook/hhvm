<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

async function f(): Awaitable<bool> {
  return true;
}

async function test(): Awaitable<void> {
  // error because old precedence means the ternary executes first.
  $result = await f() ? true : false;
  var_dump($result);
}

<<__EntryPoint>>
function main() :mixed{
  \HH\Asio\join(test());
}
