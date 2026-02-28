<?hh

namespace {
    function bar() :mixed{}

    use function foo\bar;
}

namespace {
    <<__EntryPoint>> function main(): void {
    echo "Done";
    }
}
