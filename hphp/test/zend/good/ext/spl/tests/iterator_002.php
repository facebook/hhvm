<?hh

class RecursiceArrayIterator extends ArrayIterator implements RecursiveIterator
{
    function hasChildren()
    {
        return is_array($this->current());
    }

    function getChildren()
    {
        return new RecursiceArrayIterator($this->current());
    }
}

class CrashIterator extends FilterIterator implements RecursiveIterator
{
    function accept()
    {
        return true;
    }

    function hasChildren()
    {
        return $this->getInnerIterator()->hasChildren();
    }

    function getChildren()
    {
        return new RecursiceArrayIterator($this->getInnerIterator()->current());
    }
}
<<__EntryPoint>> function main(): void {
$array = darray[0 => 1, 2 => darray[0 => 21, 22 => varray[221, 222], 23 => varray[231]], 3 => 3];

$dir = new RecursiveIteratorIterator(new CrashIterator(new RecursiceArrayIterator($array)), RecursiveIteratorIterator::LEAVES_ONLY);

foreach ($dir as $file) {
    print "$file\n";
}

echo "===DONE===\n";
}
