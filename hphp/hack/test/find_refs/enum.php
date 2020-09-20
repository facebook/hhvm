<?hh //strict

enum Size: int {
  SMALL = 0;
  MEDIUM = 1;
  LARGE = 2;
}

function test_rename(): void {
  $size = Size::SMALL; // Find ref
}
