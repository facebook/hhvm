<?php
class test {
        function _foo() {
                throw new Exception('test');
        }
        function __call($name=null, $args=null) {
                return test::_foo();
        }
}

var_dump(register_shutdown_function(array("test","__call")));

echo "Done\n";
?>