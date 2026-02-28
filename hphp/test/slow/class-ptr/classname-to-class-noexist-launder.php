<?hh

<<__EntryPoint>>
function main(): void {
  try {
    $x = HH\classname_to_class("NoExist");
  } catch (InvalidArgumentException $ex) {
    printf("%s\n", $ex->getMessage());
  }
}
