<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function ferased<T>(): void {
  new T();
}

function freified<reify T>(): void {
  new T();
}
