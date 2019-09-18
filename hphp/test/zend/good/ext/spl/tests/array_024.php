<?hh
class ArrayObject2 extends ArrayObject {
    public function count() {
        return -parent::count();
    }
}
<<__EntryPoint>>
function main_entry(): void {
  $obj = new ArrayObject(array(1,2));
  var_dump(count($obj));
  $obj = new ArrayObject2(array(1,2));
  var_dump(count($obj));
}
