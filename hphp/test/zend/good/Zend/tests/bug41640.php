<?hh
class foo {
        const FOO = 1;
        public $x = self::FOO;
}
<<__EntryPoint>> function main() {
var_dump(get_class_vars("foo"));
}
