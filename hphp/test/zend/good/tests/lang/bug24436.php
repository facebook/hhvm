<?hh
class test {
    function __construct() {
        if (!isset($this->test[0][0])) { print "test2";}
    }
}

<<__EntryPoint>> function main(): void {
  $test1 = new test();
}
