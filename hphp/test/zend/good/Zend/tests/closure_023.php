<?hh
class foo {
    public static function bar() {
        $func = function() { echo "Done"; };
        $func();
    }
}
<<__EntryPoint>> function main() {
foo::bar();
}
