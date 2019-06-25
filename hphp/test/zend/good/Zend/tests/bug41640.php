<?hh
class foo {
        const FOO = 1;
        public $x = self::FOO;
}
<<__EntryPoint>> function main(): void {
var_dump(get_class_vars("foo"));
}
