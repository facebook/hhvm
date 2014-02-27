<?php

class str {}
class str1 extends str { public function __toString() { return "a"; } }
class str2 extends str {
  public function __toString() { throw new Exception('a'); }
}

function bar(str $k) {
  $tmp = 54;
  try {
    $y = $k->__toString();
    $tmp = 2;
  } catch (Exception $x) {
    var_dump($tmp);
  }
  var_dump($tmp);
}

bar(new str1);
bar(new str2);

