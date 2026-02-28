<?hh
class SplMaxHeap2 extends SplMaxHeap{
    public function count() :mixed{
        return -parent::count();
    }
}
<<__EntryPoint>>
function main_entry(): void {
  $obj = new SplMaxHeap();
  $obj->insert(1);
  $obj->insert(2);
  var_dump(count($obj));
  $obj = new SplMaxHeap2();
  $obj->insert(1);
  $obj->insert(2);
  var_dump(count($obj));
}
