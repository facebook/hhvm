<?php
$ai = new AppendIterator();
$i1 = new ArrayIterator([]);
$i2 = new ArrayIterator([]);
$i3 = new ArrayIterator([1,2,3]);
$i4 = new ArrayIterator([]);
$i5 = new ArrayIterator([4,5,6]);
$ai->append($i1);
$ai->append($i2);
$ai->append($i3);
$ai->append($i4);
$ai->append($i5);
foreach($ai as $val) {
  echo $val."\n";
}
