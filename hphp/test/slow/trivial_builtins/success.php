<?hh

<<\TrivialHHVMBuiltinWrapper("HH\Lib\C\any")>>
function myAny($arr): bool {
  throw new Exception("This is a test. This should have been unwrapped.");
}

<<__EntryPoint>>
function main(): void {
  $arr = vec[-5, -4, 1, 2, 3];
  var_dump(myAny($arr));
}
