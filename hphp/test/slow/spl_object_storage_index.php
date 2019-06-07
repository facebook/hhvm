<?hh
class Test extends \SplObjectStorage {
  function __construct() {
    $o1 = new StdClass;
    $o2 = new StdClass;
    $o3 = new StdClass;
    $this->attach($o1);
    $this->attach($o2);
    $this->attach($o3);

    foreach($this as $key => $val) {
      print($key);
    }
  }
}

<<__EntryPoint>>
function main_spl_object_storage_index() {
new Test();
}
