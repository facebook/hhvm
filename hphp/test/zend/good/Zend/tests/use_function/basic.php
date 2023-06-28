<?hh

namespace foo\bar {
    function baz() :mixed{
        return 'foo.bar.baz';
    }
    function qux() :mixed{
        return baz();
    }
}

namespace {
    use function foo\bar\baz, foo\bar\qux;
    <<__EntryPoint>> function main(): void {
    var_dump(baz());
    var_dump(qux());
    echo "Done\n";
    }
}
