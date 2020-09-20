<?hh

namespace foo\bar {
    const baz = 42;
    const qux = 43;
}

namespace {
    use const foo\bar\baz, foo\bar\qux;
    <<__EntryPoint>> function main(): void {
    var_dump(baz);
    var_dump(qux);
    echo "Done\n";
    }
}
