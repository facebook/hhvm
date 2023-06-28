<?hh
namespace test\ns1;

class Foo implements \SplObserver {
    function update(\SplSubject $x) :mixed{
        echo "ok\n";
    }
}

class Bar implements \SplSubject {
    function attach(\SplObserver $x) :mixed{
        echo "ok\n";
    }
    function notify() :mixed{
    }
    function detach(\SplObserver $x) :mixed{
    }
}
<<__EntryPoint>> function main(): void {
$foo = new Foo();
$bar = new Bar();
$bar->attach($foo);
$foo->update($bar);
}
