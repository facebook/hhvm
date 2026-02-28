<?hh

interface i {
    function test():mixed;
}

class foo implements i {
    function test() :mixed{
        var_dump(get_parent_class(self::class));
    }
}

class bar extends foo {
    function test_bar() :mixed{
        var_dump(get_parent_class(self::class));
    }
}
<<__EntryPoint>> function main(): void {
$bar = new bar;
$foo = new foo;

$foo->test();
$bar->test();
$bar->test_bar();

var_dump(get_parent_class($bar));
var_dump(get_parent_class($foo));
var_dump(get_parent_class("bar"));
var_dump(get_parent_class("foo"));
var_dump(get_parent_class("i"));

var_dump(get_parent_class(""));
var_dump(get_parent_class("[[[["));
var_dump(get_parent_class(" "));
var_dump(get_parent_class(new stdClass));

echo "Done\n";
}
