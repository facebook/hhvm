<?php

    $a = (object) 'a'; 
    $b = (object) 'b'; 
    $c = (object) 'c'; 

   $foo = new SplObjectStorage;
    $foo->attach($a);
    $foo->attach($b);

    $bar = new SplObjectStorage;
    $bar->attach($b);
    $bar->attach($c);

    $foo->removeAllExcept($bar);
    var_dump($foo->contains($a));
    var_dump($foo->contains($b));

?>