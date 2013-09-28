<?php

$s = new SplObjectStorage();

try {
    $s->unserialize(NULL);
} catch(UnexpectedValueException $e) {
    echo $e->getMessage();
}

?>