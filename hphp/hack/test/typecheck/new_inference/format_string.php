//// def.hack

newtype FormatString<T> = string;

//// use.php
<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

function test(
  FormatString<mixed> $f1,
  HH\FormatString<mixed> $f2,
): string {
  return $f1.$f2;
}
