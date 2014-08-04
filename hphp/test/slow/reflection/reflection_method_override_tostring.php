<?php

class MyReflectionMethod extends ReflectionMethod
{
    public function toString()
    {
        return "custom toString";
    }
}

$method = new MyReflectionMethod('MyReflectionMethod', 'toString');
echo $method->toString();
