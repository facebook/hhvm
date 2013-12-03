<?php
class A extends \SplFileInfo
{
	    public function __toString() {return ' -expected- ';}
}

$a = new A('/');

// Works
echo $a, $a->__toString(), $a->__toString() . '', "\n";

// Does not work - outputs parent::__toString()
echo $a . '', "\n";
