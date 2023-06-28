<?hh
function f($x) :mixed{
  include __FILE__;
}
<<__EntryPoint>>
function main_entry(): void {
  if (isset($x)) {
    unset($x);
    $x = debug_backtrace();
  } else {
    f(10);
    print("Did not crash\n");
  }
}
