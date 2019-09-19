<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

/* HH_FIXME[4336] */
function coerce<T>(
  TypeStructure<T> $ts,
  mixed $value,
): T {
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
