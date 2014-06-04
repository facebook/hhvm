<?php

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

$arr = array("a",
      array("ca"));
$obj = new RecursiveArrayIterator($arr);
$rit = new RecursiveArrayIteratorIterator($obj);
echo "Rewind: \n";
$rit->rewind();
echo "\nNext:\n";
$rit->next();
$rit->next();

?>
===DONE===
<?php exit(0); ?>
