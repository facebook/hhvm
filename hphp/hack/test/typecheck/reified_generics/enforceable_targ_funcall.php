<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class Erased<Te> {}
class Reified<reify Tr> {}

// Note, T is not reified. Enforceable check works independently
function f<<<__Enforceable>> Tf>(): void {}

function test(): void {
  f<Erased<int>>();
  f<Reified<int>>();
  f<Reified<Erased<int>>>();
}
