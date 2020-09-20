<?hh

namespace {
    const foo = 'foo';
}

namespace x {
    use foo as bar;
    use const foo as bar;
    <<__EntryPoint>> function main(): void {
    \var_dump(bar);
    }
}
