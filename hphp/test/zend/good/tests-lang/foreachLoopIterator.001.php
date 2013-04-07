<?php

class MealIterator implements Iterator {
	private $pos=0;
	private $myContent=array("breakfast", "lunch", "dinner");
	
	public function valid() {
		global $indent;
		echo "$indent--> " . __METHOD__ . " ($this->pos)\n";
		return $this->pos<3;
	}
	
	public function next() {
		global $indent;
		echo "$indent--> " . __METHOD__ . " ($this->pos)\n";
		return $this->myContent[$this->pos++];
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

$f = new MealIterator;
var_dump($f);

echo "-----( Simple iteration: )-----\n";
foreach ($f as $k=>$v) {
	echo "$k => $v\n";	
}

$f->rewind();

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