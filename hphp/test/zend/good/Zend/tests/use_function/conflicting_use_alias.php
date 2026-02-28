<?hh

namespace {
    function foo() :mixed{
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
