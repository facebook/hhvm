<?php
class B{
	public function process($x){
		return $x;
	}
}
class C{
	public function generate($x){
		throw new Exception;
	}
}
$b = new B;
$c = new C;
try{
	$b->process($c->generate(0));
}
catch(Exception $e){
	$c->generate(0);
}
?>