<?hh

<<__Memoize>>
async function func(?string $s) {
  throw new Exception();
}

<<__EntryPoint>>
async function main() {
  try {
    await func(__hhvm_intrinsics\launder_value(null));
  } catch (Exception $ex) {
  }
  try {
    await func(__hhvm_intrinsics\launder_value(null));
  } catch (Exception $ex) {
  }
  echo "Done\n";
}
