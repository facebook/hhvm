<?hh
class test {
        function __construct() {
                if (!($this->test[0][0] ?? false)) { print "test1\n";}
                if (!isset($this->test[0][0])) { print "test2\n";}
                if (!($this->test ?? false)) { print "test1\n";}
                if (!isset($this->test)) { print "test2\n";}
        }
}
<<__EntryPoint>> function main(): void {
$test1 = new test();
}
