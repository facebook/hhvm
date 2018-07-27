<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

function to_string(mixed $value): string {
  if ($value is float)
  {
    if ($value !== $value) {
      return 'NaN';
    }
    return (string)$value;
  }
}

var_dump(to_string(42.0));
