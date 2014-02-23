<?php

class obj1 { function heh() { echo "heh\n"; } }
class obj2 { function heh() { echo "yup\n"; } }
function stuff() {
  return array(new obj1, new obj2);
}
function main() {
  list($x, $y) = stuff();
  $x->heh();
  $y->heh();
}
main();
