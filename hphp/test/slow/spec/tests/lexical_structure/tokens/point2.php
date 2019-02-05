<?php

/*
   +-------------------------------------------------------------+
   | Copyright (c) 2015 Facebook, Inc. (http://www.facebook.com) |
   +-------------------------------------------------------------+
*/

error_reporting(-1);

class Point2
{
    public $x;
    public $y;

    public function __construct($x = 0, $y = 0)
    {
//      echo "Inside " . __METHOD__ . "\n";

        $this->x = $x;
        $this->y = $y;
    }

    public function __toString()
    {
        return '(' . $this->x . ',' . $this->y . ')';
    }
}
