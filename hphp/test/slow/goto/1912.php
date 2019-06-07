<?hh


<<__EntryPoint>>
function main_1912() {
for($i=0,$j=50;
 $i<100;
 $i++) {
  while($j--) {
 if($j==17) goto end;
 }
}
 echo 'no';
 end: echo 'yes';
}
