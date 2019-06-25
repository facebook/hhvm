<?hh
function __autoload($name) {
  echo("AUTOLOAD '$name'\n");
  eval("class $name {}");
}

class BASE {
}

interface CHILD {
}

class A extends BASE implements CHILD {
}
<<__EntryPoint>> function main(): void {
$a = new A;
var_dump(is_a($a, "B1"));
var_dump(is_a($a, "A"));
var_dump(is_a($a, "BASE"));
var_dump(is_a($a, "CHILD"));
var_dump(is_subclass_of($a, "B2"));
var_dump(is_subclass_of($a, "A"));
var_dump(is_subclass_of($a, "BASE"));
var_dump(is_subclass_of($a, "CHILD"));
var_dump(is_subclass_of("X1", "X2"));
}
