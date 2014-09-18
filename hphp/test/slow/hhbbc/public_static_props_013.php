<?php

class X        { static $x = array(); }
function go()  { X::$x[0] = 2; }
function go2() { var_dump((bool)X::$x); }

go();
go2();
