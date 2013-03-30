<?php
class X
{
        public $x;
        private function __construct($x)
        {
                $this->x = $x;
        }
}

class Y extends X
{
        static public function cheat($x)
        {
                return new Y($x);
        }
}

$y = Y::cheat(5);
echo $y->x, PHP_EOL;