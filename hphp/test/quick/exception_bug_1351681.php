<?hh
function doThrow() :mixed{ throw new Exception("blah!"); }
function foo() :mixed{
  foreach (vec[1, 2, 3] as $_) {
    doThrow();
  }
  try { echo "Hi\n"; } catch (Exception $ex) { echo "We should not reach here\n"; }
}
<<__EntryPoint>> function main(): void {
try {
  foo();
} catch (Exception $x) {
  echo "it's ok\n";
}
}
