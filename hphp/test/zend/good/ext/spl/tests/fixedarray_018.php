<?hh
class SplFixedArray2 extends SplFixedArray {
    public function count() {
        return -parent::count();
    }
}
<<__EntryPoint>>
function main_entry(): void {
  $obj = new SplFixedArray(2);
  var_dump(count($obj));
  $obj = new SplFixedArray2(2);
  var_dump(count($obj));
}
