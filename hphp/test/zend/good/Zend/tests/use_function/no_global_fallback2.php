<?hh

namespace {
    function test() :mixed{
        echo "NO!";
    }
}
namespace foo {
    use function bar\test;
    <<__EntryPoint>> function main(): void {
    test();
    }
}
