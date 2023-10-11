<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

async function gen_empty_shape(): Awaitable<shape()> {
  return shape();
}

async function test(): Awaitable<void> {
  try {
    throw new Exception();
  } catch (Exception $_) {
  } finally {
    $s = await gen_empty_shape();
    Shapes::idx($s, 'foo');
  }
}
