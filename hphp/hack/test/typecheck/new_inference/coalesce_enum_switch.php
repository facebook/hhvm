<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

enum Colour : string as string {
  PINK = 'pink';
}

function returns_optional_string(): ?string {
  return null;
}

function my_test_function7(): void {
  $colour = returns_optional_string() ?? Colour::PINK;

  switch ($colour) {
    case 'red':
      break;
    case 'black':
      break;
    case 'primary':
      break;
    case 'blue':
      break;
    case 'white':
      break;
    default:
      break;
  }
}
