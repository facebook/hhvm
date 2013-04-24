<?php
/* Prototype  : proto string serialize(mixed variable)
 * Description: Returns a string representation of variable (which can later be unserialized) 
 * Source code: ext/standard/var.c
 * Alias to functions: 
 */
/* Prototype  : proto mixed unserialize(string variable_representation)
 * Description: Takes a string representation of variable and recreates it 
 * Source code: ext/standard/var.c
 * Alias to functions: 
 */

Class A {
	private $APriv = "A.APriv";
	protected $AProt = "A.AProt";
	public $APub = "A.APub";
	
	function audit() {
		return isset($this->APriv, $this->AProt, $this->APub); 
	}		
}

Class B extends A {
	private $BPriv = "B.BPriv";
	protected $BProt = "B.BProt";
	public $BPub = "B.BPub";
	
	function audit() {
		return  parent::audit() && isset($this->AProt, $this->APub,
					 $this->BPriv, $this->BProt, $this->BPub); 
	}	
}

Class C extends B {
	private $APriv = "C.APriv";
	protected $AProt = "C.AProt";
	public $APub = "C.APub";
	
	private $CPriv = "C.CPriv";
	protected $CProt = "C.BProt";
	public $CPub = "C.CPub";
	
	function audit() {
		return parent::audit() && isset($this->APriv, $this->AProt, $this->APub, 
					 $this->BProt, $this->BPub, 
					 $this->CPriv, $this->CProt, $this->CPub); 
	}
}

function prettyPrint($obj) {
	echo "\n\nBefore serialization:\n";
	var_dump($obj);

	echo "Serialized form:\n";
	$ser = serialize($obj);
	$serPrintable = str_replace("\0", '\0', $ser);
	var_dump($serPrintable);
	
	echo "Unserialized:\n";
	$uobj = unserialize($ser);
	var_dump($uobj);
	
	echo "Sanity check: ";
	var_dump($uobj->audit());
}

echo "-- Test instance of A --\n";
prettyPrint(new A);
echo "\n\n-- Test instance of B --\n";
prettyPrint(new B);
echo "\n\n-- Test instance of C --\n";
prettyPrint(new C);

echo "Done";
?>