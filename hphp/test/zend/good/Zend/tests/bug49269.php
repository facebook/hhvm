<?hh
class TestObject implements Iterator
{
    public $n = 0;
    function valid()
:mixed    {
        return ($this->n < 3);
    }
    function current() :mixed{return $this->n;}
    function next() :mixed{$this->n++;}
    function key() :mixed{ }
    function rewind() :mixed{$this->n = 0;}
}

<<__EntryPoint>> function main(): void {
$array_object = new TestObject();

foreach ((true ? $array_object : $array_object) as $item) echo "$item\n";
}
