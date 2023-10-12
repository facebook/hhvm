<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

class Ref<T> {
  public function __construct(public T $value) {}
}

async function test_union(
  bool $b,
  Awaitable<int> $g,
  ?Awaitable<string> $h,
): Awaitable<void> {
  $v = Vector {};
  $v[] = $g;
  $v[] = $h;
  await $v[0];
}

/*
async function test_option(?Awaitable<void> $g): Awaitable<void> {
  $r = new Ref($g);
  await $r->value;
}
*/

async function test_null(): Awaitable<void> {
  $r = new Ref(null);
  await $r->value;
}

async function test_dynamic(dynamic $x): Awaitable<void> {
  $r = new Ref($x);
  await $r->value;
}
