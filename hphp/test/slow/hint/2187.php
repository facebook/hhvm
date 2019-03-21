<?php

class bool{
}
class boolean{
}
class int{
}
class integer{
}
class double{
}
class float{
}
class real{
}
class string {
}
function foo(bool $b1, boolean $b2,             int $i1, integer $i2,             double $d1, float $d2, real $d3,             string $s) {
  var_dump(array($b1, $b2, $i1, $i2, $d1, $d2, $d3, $s));
}

<<__EntryPoint>>
function main_2187() {
foo(new bool, new boolean,    new int, new integer,    new double, new float, new real,    new string);
}
