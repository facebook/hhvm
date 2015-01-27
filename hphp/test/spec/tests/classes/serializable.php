<?php

/*
   +-------------------------------------------------------------+
   | Copyright (c) 2014 Facebook, Inc. (http://www.facebook.com) |
   +-------------------------------------------------------------+
*/

error_reporting(-1);

class Point implements Serializable
{
    private static $nextId = 1;

    private $x;
    private $y;
    private $id;

    public function __construct($x = 0, $y = 0)
    {
        $this->x = $x;
        $this->y = $y;
        $this->id = self::$nextId++;

        echo "\nInside " . __METHOD__ . ", $this\n\n";
    }

    public function __toString()
    {
        return 'ID:' . $this->id . '(' . $this->x . ',' . $this->y . ')';
    }   

    public function serialize()
    {
        echo "\nInside " . __METHOD__ . ", $this\n\n";
        
        return serialize(array('y' => $this->y, 'x' => $this->x));
    }

    public function unserialize($data)
    {
        $data = unserialize($data);
        $this->x = $data['x'];
        $this->y = $data['y'];
        $this->id = self::$nextId++;

        echo "\nInside " . __METHOD__ . ", $this\n\n";
    }
}

echo "---------------- create, serialize, and unserialize a Point -------------------\n";

$p = new Point(2, 5);
echo "Point \$p = $p\n";

$s = serialize($p);
var_dump($s);

echo "------\n";

$v = unserialize($s);
var_dump($v);

echo "------\n";

class ColoredPoint extends Point implements Serializable
{
    const RED = 1;
    const BLUE = 2;

    private $color;

    public function __construct($x = 0, $y = 0, $color = RED)
    {
        parent::__construct($x, $y);
        $this->color = $color;

        echo "\nInside " . __METHOD__ . ", $this\n\n";
    }

    public function __toString()
    {
        return parent::__toString() . $this->color;
    }   

    public function serialize()
    {
        echo "\nInside " . __METHOD__ . ", $this\n\n";
        
        return serialize(array(
            'color' => $this->color,
            'baseData' => parent::serialize()
        ));
    }

    public function unserialize($data)
    {
        $data = unserialize($data);
        $this->color = $data['color'];
        parent::unserialize($data['baseData']);

        echo "\nInside " . __METHOD__ . ", $this\n\n";
    }
}

echo "---------------- Serialize ColoredPoint -------------------\n";

$cp = new ColoredPoint(9, 8, ColoredPoint::BLUE);
echo "ColoredPoint \$cp = $cp\n";

$s = serialize($cp);
var_dump($s);

$v = unserialize($s);
var_dump($v);

echo "---------------- end -------------------\n";
