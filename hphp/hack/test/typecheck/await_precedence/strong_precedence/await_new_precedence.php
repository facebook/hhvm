<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

class AwaitPrecedenceTestClass{
  public async function bar() : Awaitable<void> { }
}

async function foo(Awaitable<bool> $a, Awaitable<bool> $b, Awaitable<bool> $c)
  : Awaitable<void> {
  await $a ? $b : $c;
}

async function bar(AwaitPrecedenceTestClass $a) : Awaitable<void> {
  await $a->bar();
}

function id<T>(Awaitable<T> $a) : Awaitable<T> { return $a; }

async function baz(Awaitable<int> $a) : Awaitable<void> {
  await $a |> id($$) |> id($$);
}
