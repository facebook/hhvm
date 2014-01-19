<?php
class C extends ArrayObject {
	public $p = 'object property';
}	

function access_p($ao) {
  // isset
  var_dump(isset($ao->p));
  // read
  var_dump($ao->p);
  // write  
  $ao->p = $ao->p . '.changed';
  var_dump($ao->p);
}

$ao = new C(array('p'=>'array element'));
$ao->setFlags(ArrayObject::ARRAY_AS_PROPS);

echo "\n--> Access the real property:\n";
access_p($ao);

echo "\n--> Remove the real property and access the array element:\n";
unset($ao->p);
access_p($ao);

echo "\n--> Remove the array element and try access again:\n";
unset($ao->p);
access_p($ao);
?>