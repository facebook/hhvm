<?hh
class foo {
    public function foo() {}
}

class bar extends foo {
}
<<__EntryPoint>> function main(): void {
print_r(get_class_methods("bar"));
}
