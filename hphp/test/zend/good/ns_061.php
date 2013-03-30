<?php
class A {}
use \A as B;
echo get_class(new B)."\n";