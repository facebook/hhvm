<?php
class Foo implements IteratorAggregate
{
    public function bar() {
        throw new Exception;
    }
					        
    public function getIterator() {
        return new ArrayIterator($this->bar());
    }
}
var_dump(iterator_to_array(new Foo));
?>