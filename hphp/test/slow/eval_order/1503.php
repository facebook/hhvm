<?php

class X {
}
 function foo() {
 var_dump('foo');
}
 $x = new X;
 unset($x->a[foo()]->y);

