<?hh

namespace {
    function foo() {
        return 'foo';
    }
}

namespace x {
    use foo as bar;
    use function foo as bar;
    <<__EntryPoint>> function main(): void {
    \var_dump(bar());
    }
}
