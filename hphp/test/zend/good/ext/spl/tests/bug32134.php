<?php
	
class myArray extends ArrayIterator
{

    public function __construct($array = array())
    {
        parent::__construct($array);
    }

  private static $offsetGetI = 0;

    public function offsetGet($index)
    {
        echo __METHOD__ . "($index)\n";
        if (++self::$offsetGetI > 3) exit(1);
        return parent::offsetGet($index);
    }

    public function offsetSet($index, $newval)
    {
        echo __METHOD__ . "($index,$newval)\n";
        return parent::offsetSet($index, $newval);
    }

}

$myArray = new myArray();

$myArray->offsetSet('one', 'one');
var_dump($myArray->offsetGet('one'));

$myArray['two'] = 'two';
var_dump($myArray['two']);

echo "===DONE===\n";
