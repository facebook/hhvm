<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

async function test(bool $b, Awaitable<int> $a): Awaitable<int> {
  return await $b ? $a : null;
}
