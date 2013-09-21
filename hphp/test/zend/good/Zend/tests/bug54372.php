<?php
class test_class
{
    public function __get($name)
    {
        return $this;
    }

    public function b()
    {
        echo "ok\n";
    }
}

global $test3;
$test3 = new test_class();
$test3->a->b();
?>