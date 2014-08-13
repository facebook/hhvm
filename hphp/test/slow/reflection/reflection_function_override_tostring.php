<?php

class MyReflectionFunction extends ReflectionFunction
{
    public function toString()
    {
        return 'custom toString';
    }
}

$function = new MyReflectionFunction('str_replace');
echo $function->toString();
