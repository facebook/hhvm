<?php

function autoload_first($name)
{
  echo __METHOD__ . "\n";
  throw new Exception('first');
}

function autoload_second($name)
{
  echo __METHOD__ . "\n";
  throw new Exception('second');
}

spl_autoload_register('autoload_first');
spl_autoload_register('autoload_second');

try {
    class_exists('ThisClassDoesNotExist');
} catch(Exception $e) {
    do {
        echo $e->getMessage()."\n";
    } while($e = $e->getPrevious());
}

try {
    new ThisClassDoesNotExist;
} catch(Exception $e) {
    do {
        echo $e->getMessage()."\n";
    } while($e = $e->getPrevious());
}

class_exists('ThisClassDoesNotExist');
?>
===DONE===