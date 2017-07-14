<?php
class Test {
	public function test(){
		$arr = (object) [
			'children' => []
		];
		$arr->children[] = 1;
		return $arr;
	}
}

$o = new Test();
$o->test();

print_r($o->test());
?>
