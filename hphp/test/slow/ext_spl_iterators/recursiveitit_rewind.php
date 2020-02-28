<?hh

class RecItIt extends RecursiveIteratorIterator {

  public function next() {
    echo "Called next() \n";
    parent::next();
  }

  public function rewind() {
    echo "Called rewind() \n";
    parent::rewind();
  }

}



<<__EntryPoint>>
function main_recursiveitit_rewind() {
$rii = new RecItIt(new RecursiveArrayIterator(varray[1,2,3,4]));
$rii->rewind();
}
