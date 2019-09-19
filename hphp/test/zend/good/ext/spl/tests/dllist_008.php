<?hh
class SplDoublyLinkedList2 extends SplDoublyLinkedList{
    public function count() {
        return -parent::count();
    }
}
<<__EntryPoint>>
function main_entry(): void {
  $obj = new SplDoublyLinkedList();
  $obj[] = 1;
  $obj[] = 2;
  var_dump(count($obj));
  $obj = new SplDoublyLinkedList2();
  $obj[] = 1;
  $obj[] = 2;
  var_dump(count($obj));
}
