<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

function foo(
  vec<arraykey> $parts,
): string {
  return "A";
}
function bar(arraykey ...$args): string {
  return foo(vec($args));
}
