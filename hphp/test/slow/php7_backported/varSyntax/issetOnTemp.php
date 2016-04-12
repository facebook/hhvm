<?php
var_dump(isset([0, 1][0]));
var_dump(isset(([0, 1] + [])[0]));
var_dump(isset([[0, 1]][0][0]));
var_dump(isset(([[0, 1]] + [])[0][0]));
var_dump(isset(((object) ['a' => 'b'])->a));
//var_dump(isset(['a' => 'b']->a));
//var_dump(isset("str"->a));
var_dump(isset((['a' => 'b'] + [])->a));
var_dump(isset((['a' => 'b'] + [])->a->b));
