<?hh

class RecursiveArrayIteratorIterator extends RecursiveIteratorIterator
{
  function rewind()
  {
    echo __METHOD__ . " called \n";
    parent::rewind();
  }
  function beginChildren()
  {
    echo __METHOD__ . " called \n";
  }

  function endChildren()
  {
    echo __METHOD__ . " called \n";
  }

}
<<__EntryPoint>> function main(): void {
$arr = varray["a",
      varray["ca"]];
$obj = new RecursiveArrayIterator($arr);
$rit = new RecursiveArrayIteratorIterator($obj);
echo "Rewind: \n";
$rit->rewind();
echo "\nNext:\n";
$rit->next();
$rit->next();

echo "===DONE===\n";
}
