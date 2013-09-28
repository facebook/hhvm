<?php

function foo() {
 goto a;
 b: echo 'Foo';
 return;
a: echo 'Bar';
 goto b;
}
 foo();
