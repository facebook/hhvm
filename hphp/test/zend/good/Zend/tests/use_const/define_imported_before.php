<?hh

namespace {
    const bar = 42;

    use const foo\bar;
}

namespace {
    <<__EntryPoint>> function main(): void {
    echo "Done";
    }
}
