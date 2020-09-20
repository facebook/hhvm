<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

class A<Ta, reify Tb> {}

function f(): void {
  3 as A<_, A<_, _>>;
}
