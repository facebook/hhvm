<?php
class test {
        function __construct() {
                if (empty($this->test[0][0])) { print "test1\n";}
                if (!isset($this->test[0][0])) { print "test2\n";}
                if (empty($this->test)) { print "test1\n";}
                if (!isset($this->test)) { print "test2\n";}
        }
}

$test1 = new test();
?>