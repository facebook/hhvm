<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function contains<T>(
  Traversable<T> $traversable,
  T $value,
): bool {
  return false;
}
function minimal_recreation(): void {
  $setOfStrings = Set { 'hello', 'world' };
  contains($setOfStrings, 33);            // line 5
  minimal_recreation_helper($setOfStrings); // line 6
}
function minimal_recreation_helper(Set<string> $set_of_strings_param): void {
  $set_of_strings_param;
}
