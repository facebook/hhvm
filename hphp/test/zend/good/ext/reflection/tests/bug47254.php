<?php
class A
{
	protected function a() {}
	
}

class B extends A
{
	public function b() {}
}

$B = new B();
$R = new ReflectionObject($B);
$m = $R->getMethods();
print_r($m);

?>
