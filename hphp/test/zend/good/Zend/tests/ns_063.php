<?hh
namespace Foo;
class Bar {
    function Bar() :mixed{
        echo "ok\n";
    }
}
<<__EntryPoint>> function main(): void {
new Bar();
echo "ok\n";
}
