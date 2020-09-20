<?hh
interface foobar {
    function __construct();
}
abstract class bar implements foobar {
    public function __construct($x = 1) {
    }
}
final class foo extends bar implements foobar {
    public function __construct($x = 1, $y = 2) {
    }
}
<<__EntryPoint>> function main(): void {
new foo;
print "ok!";
}
