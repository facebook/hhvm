<?hh

class MyCollection extends ArrayObject
{
    public function add($dataArray)
    {
        foreach($dataArray as $value) $this->append($value);
    }

    public function offsetSet($index, $value)
    {
        parent::offsetSet($index, $value);
    }
}
<<__EntryPoint>> function main(): void {
$data1=array('one', 'two', 'three');
$data2=array('four', 'five');

$foo=new MyCollection($data1);
$foo->add($data2);

print_r($foo->getArrayCopy());

echo "Done\n";
}
