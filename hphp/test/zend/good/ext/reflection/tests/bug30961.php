<?php
    class a
    {
    }

    class b extends a
    {
    }

    $ref1 = new ReflectionClass('a');
    $ref2 = new ReflectionClass('b');
    echo $ref1->getStartLine() . "\n";
    echo $ref2->getStartLine() . "\n";
?>