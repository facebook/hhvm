<?php
$obj = new SplDoublyLinkedList();
$obj[] = 1;
$obj[] = 2;
var_dump(count($obj));
class SplDoublyLinkedList2 extends SplDoublyLinkedList{
    public function count() {
        return -parent::count();
    }
}
$obj = new SplDoublyLinkedList2();
$obj[] = 1;
$obj[] = 2;
var_dump(count($obj));
?>