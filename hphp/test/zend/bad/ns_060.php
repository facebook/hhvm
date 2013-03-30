<?php
namespace Foo;
use Bar\A as B;
class A {}
$a = new B;
$b = new A;
echo get_class($a)."\n";
echo get_class($b)."\n";
namespace Bar;
use Foo\A as B;
$a = new B;
$b = new A;
echo get_class($a)."\n";
echo get_class($b)."\n";
class A {}