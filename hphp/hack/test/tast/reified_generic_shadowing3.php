<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

function f<T>(): void {
  new T(); // T
  new U(); // \U
}
