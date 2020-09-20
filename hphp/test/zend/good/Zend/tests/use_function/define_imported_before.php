<?hh

namespace {
    function bar() {}

    use function foo\bar;
}

namespace {
    <<__EntryPoint>> function main(): void {
    echo "Done";
    }
}
