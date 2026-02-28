<?hh
class foo {
    public static function bar() :mixed{
        $func = function() { echo "Done"; };
        $func();
    }
}
<<__EntryPoint>> function main(): void {
foo::bar();
}
