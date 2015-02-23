<?php

//https://github.com/facebook/hhvm/issues/4846

// The namespace is part of the bug
// remove it and the bug disappears
namespace HHVMTest\BuggyStaticClass;

class BuggyExtractClass
{
    public static function main()
    {
        $growthLimit = 1024 * 1024;
        $baseMemory  = memory_get_usage();
        for ( $i = 0; $i < 100000 ; $i++ )
        {
            static::modifyRow(['value' => 1, 'value2' => 'something else', 'value3' => 2.3]);
            static::modifyRow(['value' => 1, 'value2' => 'something else', 'value3' => 2.3]);
            static::modifyRow(['value' => 1, 'value2' => 'something else', 'value3' => 2.3]);
            if ( memory_get_usage() > ( $baseMemory + $growthLimit ) )
            {
                echo "Memory usage growing\n";
                return;
            }
        }
        echo "Usage remained flat\n";
    }

    protected static function modifyRow($args)
    {
        // Memory is lost here
        extract($args);
    }
}

BuggyExtractClass::main();
