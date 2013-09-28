<?php

function foo() {
 goto a;
 echo 'Foo';
 a: echo 'Bar';
}
 foo();
