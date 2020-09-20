<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

function f<reify T>(): void {
  new T();
}
