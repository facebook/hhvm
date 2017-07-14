<?php
class bar {
        public $foo = 1;
        public $bar = 1;

        function __destruct() {
                throw $this->foo;
        }
}
foreach (new bar as &$foo) {
        try {
                $foo = new Exception;
                return; // frees the loop variable
        } catch (Exception $e) {
                echo "exception\n";
        }
}
echo "end\n";
?>
