<?hh
class Test { }
<<__EntryPoint>> function main(): void {
var_dump(get_parent_class('Test'));
$t = new Test;
var_dump(get_parent_class($t));
}
