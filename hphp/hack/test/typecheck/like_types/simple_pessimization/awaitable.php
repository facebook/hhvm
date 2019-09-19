<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

async function trust(): Awaitable<int> {
  return 4;
}

function untrust(): Awaitable<int> {
  return trust();
}

async function test(): Awaitable<int> {
  $trust = await trust(); // TODO(52747109)
  hh_show($trust);

  $untrust = untrust();
  hh_show($untrust);

  return 0;
}
