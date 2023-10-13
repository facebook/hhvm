<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

enum SwitchEnum: string {
  A = 'a';
  B = 'b';
}

function default_first(SwitchEnum $enum): int {
  $box = null;
  switch ($enum) {
    default:
    case SwitchEnum::A:
      $box = 1;
      break;
  }
  return $box;
}

function default_last(SwitchEnum $enum): int {
  $box = null;
  switch ($enum) {
    case SwitchEnum::A:
    default:
      $box = 1;
      break;
  }
  return $box;
}
