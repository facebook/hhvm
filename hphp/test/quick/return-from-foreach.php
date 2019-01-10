<?hh
function f1() {
  $arr = array(1,2,3);
  foreach ($arr as $v1) {
    foreach ($arr as $v2) {
      return;
    }
  }
}
f1();
echo "Done\n";


