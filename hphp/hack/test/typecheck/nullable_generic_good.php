<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function coerce<T>(
  TypeStructure<T> $ts,
  mixed $value,
): T {
  throw new Exception();
}

function getNullableParamOfType<T>(
  TypeStructure<T> $ts,
  mixed $value
): ?T {
  if ($value === null) {
    return null;
  }
  return coerce($ts, $value);
}
