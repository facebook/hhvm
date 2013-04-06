<?php
class setter {
	public $n;
	public $x = array('a' => 1, 'b' => 2, 'c' => 3);

	function __get($nm) {
		echo "Getting [$nm]\n";

		if (isset($this->x[$nm])) {
			$r = $this->x[$nm];
			echo "Returning: $r\n";
			return $r;
		} 
		else {
			echo "Nothing!\n";
		}
	}

	function __set($nm, $val) {
		echo "Setting [$nm] to $val\n";
                    
		if (isset($this->x[$nm])) {
			$this->x[$nm] = $val;
			echo "OK!\n";
		} 
		else {
			echo "Not OK!\n";
		}
	}
}

$foo = new Setter();

// this doesn't go through __set()... should it?
$foo->n = 1;

// the rest are fine...
$foo->a = 100;
$foo->a++;
$foo->z++;
var_dump($foo);
        
?>