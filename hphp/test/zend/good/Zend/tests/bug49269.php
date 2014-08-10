<?php
class TestObject implements Iterator
{
    public $n = 0;
    function valid()
    {
        return ($this->n < 3);
    }
    function current() {return $this->n;}
    function next() {$this->n++;}
    function key() { }
    function rewind() {$this->n = 0;}
}


$array_object = new TestObject();

foreach ((true ? $array_object : $array_object) as $item) echo "$item\n";
?>
