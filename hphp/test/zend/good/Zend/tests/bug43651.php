<?hh
class Test {
    static function foo() {}
}
<<__EntryPoint>> function main(): void {
var_dump(is_callable("\\\\"));
var_dump(is_callable("\\"));
var_dump(is_callable("x\\"));
var_dump(is_callable("\\x"));
var_dump(is_callable("x\\x"));
var_dump(is_callable("x\\\\"));
var_dump(is_callable("\\x"));
var_dump(is_callable("x\\\\x"));
var_dump(is_callable("cd"));
var_dump(is_callable("Test\\"));
var_dump(is_callable("\\Test"));
var_dump(is_callable("\\Test\\"));
var_dump(is_callable("Test::foo"));
var_dump(is_callable("\\Test::foo"));
var_dump(is_callable("is_string"));
var_dump(is_callable("\\is_string"));
}
