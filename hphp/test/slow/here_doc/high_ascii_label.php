<?hh

<<__EntryPoint>>
function main_high_ascii_label() {
  eval("function foo() { return <<<\xff\nXYZ\n\xff\n; }");
 var_dump(foo());
 eval("function bar() { return <<<'\xff'\nXYZ\n\xff\n; }");
 var_dump(bar());
}
