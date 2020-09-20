<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

type Shape0 = shape();
type Shape1 = shape(?'x' => int);
type Shape2 = shape(?'x' => int, ?'y' => string);

function expect<T>(T $x):void { }

function testshapes(
  Shape0 $s0,
  Shape1 $s1,
  Shape2 $s2,
):void {
  // Let's just confirm that Shape0 <: Shape1 <: Shape2
  expect<Shape2>($s0);
  expect<Shape2>($s1);
  expect<Shape1>($s0);
  // All of these work
  $x2 = $s2['x'] ?? 5;
  $x1 = $s1['x'] ?? 5;
  $y2 = $s2['y'] ?? "a";
  $x0 = $s0['x'] ?? 5;
  $y0 = $s0['y'] ?? "a";
  $y1 = $s1['y'] ?? "a";
}
