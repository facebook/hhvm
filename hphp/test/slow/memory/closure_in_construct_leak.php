<?php

// https://github.com/facebook/hhvm/issues/4863

class CallbackBucket
{
    public $callbacks = [];

    public function __construct()
    {
        $this->callbacks[] = function() {  return "Hello"; };
        $this->callbacks[] = function() {  return "Hello"; };
        $this->callbacks[] = function() {  return "Hello"; };
    }
}

function main()
{
    $growthLimit = 1024 * 1024;
    $baseMemory  = memory_get_usage();
    for ( $i = 0; $i < 1000000 ; $i++ )
    {
        $object = new CallbackBucket();
        if ( memory_get_usage() > ( $baseMemory + $growthLimit ) )
        {
            echo "Memory usage growing\n";
            return;
        }
    }
    echo "Usage remained flat\n";
}

main();
