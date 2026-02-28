<?hh

namespace {
    const foo = 'foo.const';
    function foo() :mixed{
        return 'foo.function';
    }
}

namespace x {
    use const foo as bar;
    use function foo as bar;
    <<__EntryPoint>> function main(): void {
    \var_dump(bar);
    \var_dump(bar());
    }
}
