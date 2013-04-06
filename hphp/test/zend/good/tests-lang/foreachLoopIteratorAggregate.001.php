<?php
class EnglishMealIterator implements Iterator {
	private $pos=0;
	private $myContent=array("breakfast", "dinner", "tea");
	
	public function valid() {
		global $indent;
		echo "$indent--> " . __METHOD__ . " ($this->pos)\n";
		return $this->pos < count($this->myContent);
	}
	
	public function next() {
		global $indent;
		echo "$indent--> " . __METHOD__ . " ($this->pos)\n";
		$this->pos++;
	}
	
	public function rewind() {
		global $indent;
		echo "$indent--> " . __METHOD__ . " ($this->pos)\n";
		$this->pos=0;
	}

	public function current() {
		global $indent;
		echo "$indent--> " . __METHOD__ . " ($this->pos)\n";
		return $this->myContent[$this->pos];
	}
	
	public function key() {
		global $indent;
		echo "$indent--> " . __METHOD__ . " ($this->pos)\n";
		return "meal " . $this->pos;
	}
	
}

class FrenchMealIterator implements Iterator {
	private $pos=0;
	private $myContent=array("petit dejeuner", "dejeuner", "gouter", "dinner");
	
	public function valid() {
		global $indent;
		echo "$indent--> " . __METHOD__ . " ($this->pos)\n";
		return $this->pos < count($this->myContent);
	}
	
	public function next() {
		global $indent;
		echo "$indent--> " . __METHOD__ . " ($this->pos)\n";
		$this->pos++;
	}
	
	public function rewind() {
		global $indent;
		echo "$indent--> " . __METHOD__ . " ($this->pos)\n";
		$this->pos=0;
	}

	public function current() {
		global $indent;
		echo "$indent--> " . __METHOD__ . " ($this->pos)\n";
		return $this->myContent[$this->pos];
	}
	
	public function key() {
		global $indent;
		echo "$indent--> " . __METHOD__ . " ($this->pos)\n";
		return "meal " . $this->pos;
	}
	
}


Class EuropeanMeals implements IteratorAggregate {
	
	private $storedEnglishMealIterator;
	private $storedFrenchMealIterator;
	
	public function __construct() {
		$this->storedEnglishMealIterator = new EnglishMealIterator;
		$this->storedFrenchMealIterator = new FrenchMealIterator;
	}
	
	public function getIterator() {
		global $indent;
		echo "$indent--> " . __METHOD__  . "\n";
		
		//Alternate between English and French meals
		static $i = 0;
		if ($i++%2 == 0) {
			return $this->storedEnglishMealIterator;
		} else {
			return $this->storedFrenchMealIterator;
		}
	}
	
}

$f = new EuropeanMeals;
var_dump($f);

echo "-----( Simple iteration 1: )-----\n";
foreach ($f as $k=>$v) {
	echo "$k => $v\n";	
}
echo "-----( Simple iteration 2: )-----\n";
foreach ($f as $k=>$v) {
	echo "$k => $v\n";	
}


$indent = " ";
echo "\n\n\n-----( Nested iteration: )-----\n";
$count=1;
foreach ($f as $k=>$v) {
	echo "\nTop level "  .  $count++ . ": \n"; 
	echo "$k => $v\n";
	$indent = "     ";
	foreach ($f as $k=>$v) {
		echo "     $k => $v\n";	
	}
	$indent = " ";
}


?>
===DONE===