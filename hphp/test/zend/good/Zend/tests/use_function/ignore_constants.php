<?hh

namespace foo {
    const bar = 42;
}

namespace {
    const bar = 43;
}

namespace {
    use function foo\bar;
    <<__EntryPoint>> function main(): void {
    var_dump(bar);
    echo "Done\n";
    }
}
