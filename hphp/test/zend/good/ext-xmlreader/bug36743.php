<?php

class Test extends XMLReader
{
    private $testArr = array();
    public function __construct()
    {
        $this->testArr[] = 1;
        var_dump($this->testArr);
    }
}

$t = new test;

echo "Done\n";
?>