<?hh

<<__EntryPoint>>
function main_append_iterator_handles_empty_iterators() {
$ai = new AppendIterator();
$i1 = new ArrayIterator(varray[]);
$i2 = new ArrayIterator(varray[]);
$i3 = new ArrayIterator(varray[1,2,3]);
$i4 = new ArrayIterator(varray[]);
$i5 = new ArrayIterator(varray[4,5,6]);
$ai->append($i1);
$ai->append($i2);
$ai->append($i3);
$ai->append($i4);
$ai->append($i5);
foreach($ai as $val) {
  echo $val."\n";
}
}
