<?hh
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


<<__EntryPoint>>
function main_same_float() :mixed{
var_dump(to_string(42.0));
}
