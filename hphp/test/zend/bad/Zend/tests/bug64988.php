<?php
abstract class Base1 {
	public function insert(array $data){
		return array_reverse($data);
	}
}

class Noisy1 extends Base1 {
	public function insert(array $data, $option1 = Null) {
		if (!empty($option1)) {
			$data['option1'] = $option1;
		}
		return parent::insert($data);
	}
}
class Smooth1 extends Noisy1 {
	public function insert(array $data) {
		return parent::insert($data, count($data));
	}
}

$o = new Smooth1();
echo "okey";
?>