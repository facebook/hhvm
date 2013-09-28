<?php

class B {
 const X='xxx';
 }
class C {
 const Y=B::X;
 }
var_dump(get_class_constants('C'));
