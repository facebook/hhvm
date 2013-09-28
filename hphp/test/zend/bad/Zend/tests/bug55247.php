<?php
class Test{
    public static function __callStatic($method, $arguments)
	{
        echo $method . PHP_EOL;
    }
    public function __call($method, $arguments) 
	{
        echo $method . PHP_EOL;
    }
}

$method = 'method';

$test = new Test();

$test->method();
$test->$method();
$test->{'method'}();

Test::method();
Test::$method();
Test::{'method'}();