<?php
class Foo {
	const A = self::B;
}
echo Foo::A."\n";
?>