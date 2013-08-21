<?php

try {
    $s = new SplObjectStorage();
    $s->unserialize('s:31:"something which is not an array";');
} catch (UnexpectedValueException $e){
    var_dump($e->getMessage());
}

var_dump($s->count() == 0);

echo "===RUN2===\n";

try {
    $s = new SplObjectStorage();
    $s->unserialize('s:60:"something broken";');
} catch (UnexpectedValueException $e){
    var_dump($e->getMessage());
}

var_dump($s->count() == 0);
?>
