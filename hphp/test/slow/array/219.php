<?hh


<<__EntryPoint>>
function main_219() {
$a = varray[1, 2];
 foreach ($a as $item) {
   print 'A['.$item.']';
   if ($item == 1) $a[] = 'new item';
 }
 foreach ($a as $item) {
   print 'B['.$item.']';
 }
var_dump($a);
}
