<?hh

class RecursiveArrayIteratorIterator extends RecursiveIteratorIterator
{
    function rewind()
    {
        echo "<ul>\n";
        parent::rewind();
    }
    function beginChildren()
    {
        echo str_repeat('  ',$this->getDepth())."<ul>\n";
    }

    function endChildren()
    {
        echo str_repeat('  ',$this->getDepth())."</ul>\n";
    }
    function valid()
    {
        if (!parent::valid()) {
            echo "<ul>\n";
            return false;
        }
        return true;
    }
}
<<__EntryPoint>> function main(): void {
$arr = varray["a", varray["ba", varray["bba", "bbb"], varray[varray["bcaa"]]], varray["ca"], "d"];
$obj = new RecursiveArrayIterator($arr);
$rit = new RecursiveArrayIteratorIterator($obj);
foreach($rit as $k=>$v)
{
    echo str_repeat('  ',$rit->getDepth()+1)."$k=>$v\n";
}
echo "===DONE===\n";
}
