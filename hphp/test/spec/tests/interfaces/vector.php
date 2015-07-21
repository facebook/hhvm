<?php

/*
   +-------------------------------------------------------------+
   | Copyright (c) 2015 Facebook, Inc. (http://www.facebook.com) |
   +-------------------------------------------------------------+
*/

error_reporting(-1);

class MyVector implements ArrayAccess
{
    private $elements;

    public function __construct($p1)
    {
        $this->elements = $p1;
    }

    public function offsetExists($offset)
    {
        echo "Inside " . __METHOD__ . " with offset >$offset<\n";
        
        return isset($this->elements[$offset]);
    }

    public function offsetSet($offset, $value)
    {
        echo "Inside " . __METHOD__ . " with offset >$offset<\n";
        
        if (is_null($offset))
        {
            $this->elements[] = $value;
        }
        else
        {
            $this->elements[$offset] = $value;
        }
    }

    public function offsetGet($offset)
    {
        echo "Inside " . __METHOD__ . " with offset >$offset<\n";

        if (isset($this->elements[$offset]))
        {
            return $this->elements[$offset];
        }
        else
        {
            return NULL;
        }
    }

    public function offsetUnset($offset)
    {
        echo "Inside " . __METHOD__ . " with offset >$offset<\n";
        
        unset($this->elements[$offset]);
    }
}

echo "--------------------\n";

$vect1 = new MyVector(array(10, 'A' => 2.3, "up"));
//var_dump($vect1);

//var_dump($vect1->offsetExists(10));
$vect1[10] = 987;       // calls Vector::offsetSet(10, 987)
//var_dump($vect1->offsetExists(10));
//var_dump($vect1->offsetExists(1));
var_dump($vect1[1]);    // calls Vector::offsetGet(1), retrieving "up"

$vect1[] = "xxx";   // calls Vector::offsetSet(11, "xxx")
var_dump($vect1);
