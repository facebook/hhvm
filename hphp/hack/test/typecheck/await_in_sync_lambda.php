//// file1.php
<?hh

async function gen(): Awaitable<void> {}

//// file2.php
<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function call((function(): void) $f): void {
  $f();
}

function test(): void {
  call(() ==> await gen());
}
