<?hh

namespace foo {
    function baz() :mixed{
        return 'foo.baz';
    }
}

namespace bar {
    function baz() :mixed{
        return 'bar.baz';
    }
}

namespace {
    use function foo\baz as foo_baz,
                 bar\baz as bar_baz;
    <<__EntryPoint>> function main(): void {
    var_dump(foo_baz());
    var_dump(bar_baz());
    echo "Done\n";
    }
}
