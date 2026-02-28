<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class C<reify Ta, reify Tb> {}
function f<reify T>(): void {}

function f_nestedArity(): void {
  f<C<int, C<int>>>(); // bad
  f<C<int, C<int, string>>>();
}

function f_missingKeyword(): void {
  f<C<int, C<int, string>>>();
}
