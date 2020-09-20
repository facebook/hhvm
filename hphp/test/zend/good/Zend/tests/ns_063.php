<?hh
namespace Foo;
class Bar {
    function Bar() {
        echo "ok\n";
    }
}
<<__EntryPoint>> function main(): void {
new Bar();
echo "ok\n";
}
