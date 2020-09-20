<?hh

namespace foo {
    const baz = 42;
}

namespace bar {
    const baz = 43;
}

namespace {
    use const foo\baz as foo_baz,
              bar\baz as bar_baz;
    <<__EntryPoint>> function main(): void {
    var_dump(foo_baz);
    var_dump(bar_baz);
    echo "Done\n";
    }
}
