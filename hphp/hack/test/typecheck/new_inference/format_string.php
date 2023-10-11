//// def.hack



//// use.php
<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function test(
  HH\FormatString<mixed> $f,
): string {
  return 'asdf'.$f;
}
