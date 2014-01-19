<?php
class Foo {
	const A = self::B;
	const B = "ok";
}
echo Foo::A."\n";
?>