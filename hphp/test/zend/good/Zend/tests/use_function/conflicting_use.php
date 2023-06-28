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
    use function foo\baz, bar\baz;
    <<__EntryPoint>> function main(): void {
    echo "Done\n";
    }
}
