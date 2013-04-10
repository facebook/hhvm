<?php
class EnglishMealIterator implements Iterator {
	private $pos=0;
	private $myContent=array("breakfast", "dinner", "tea");
	
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

class A1 implements IteratorAggregate {
	function getIterator() {
		return new EnglishMealIterator;
	}
}

class A2 implements IteratorAggregate {
	function getIterator() {
		return new A1;
	}
}

class A3 implements IteratorAggregate {
	function getIterator() {
		return new A2;
	}
}

echo "\n-----( A1: )-----\n";
foreach (new A1 as $k=>$v) {
	echo "$k => $v\n";	
}

echo "\n-----( A2: )-----\n";
foreach (new A2 as $k=>$v) {
	echo "$k => $v\n";	
}

echo "\n-----( A3: )-----\n";
foreach (new A3 as $k=>$v) {
	echo "$k => $v\n";	
}

?>
===DONE===