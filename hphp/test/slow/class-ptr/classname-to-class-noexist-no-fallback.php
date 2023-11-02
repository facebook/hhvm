<?hh

<<__EntryPoint>>
function main(): void {
  try {
    // When noexist fallback-to-string is off
    // this exception message should mention lazy class
    HH\classname_to_class(NoExist::class);
  } catch (InvalidArgumentException $ex) {
    printf("%s\n", $ex->getMessage());
  }
}
