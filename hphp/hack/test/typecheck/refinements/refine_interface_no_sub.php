<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

interface IBar {}
interface IBaz {}

function foo<T as IBar>(T $x):void { }

function testit(
  IBar $bar,
): void {
  // Just because there is no subtype relationship between IBar and IBaz
  // does not mean that this will fail!
  $bar as IBaz;
  // This should produce an error; this code is reachable
  foo<string>("A");
}
