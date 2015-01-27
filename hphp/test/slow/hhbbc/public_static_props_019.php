<?php

class Asd {
  static $thing = 2;
}

function go() {
  var_dump(Asd::$thing);
  everything_bad('Asd', 'thing');
  var_dump(Asd::$thing);
}
function everything_bad($y, $z) { $y::${$z} = 'heh'; }

go();
