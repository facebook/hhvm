<?php
class Foo {
	const A = Foo::B;
	const B = Foo::A;
}
echo Foo::A;
?>