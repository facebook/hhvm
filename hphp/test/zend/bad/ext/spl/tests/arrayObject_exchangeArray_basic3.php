<?php

class C {
	public $pub1 = 'public1';
} 

echo "--> exchangeArray() with objects:\n";
$original = new C;
$ao = new ArrayObject($original);
$swapIn = new C;
try {
	$copy = $ao->exchangeArray($swapIn);
	$copy['addedToCopy'] = 'added To Copy';
} catch (Exception $e) {
	echo "Exception:" . $e->getMessage() . "\n";
}
$swapIn->addedToSwapIn = 'added To Swap-In';
$original->addedToOriginal = 'added To Original';
var_dump($ao, $original, $swapIn, $copy);


echo "\n\n--> exchangeArray() with no arg:\n";
unset($original, $ao, $swapIn, $copy);
$original = new C;
$ao = new ArrayObject($original);
try {
	$copy = $ao->exchangeArray();
	$copy['addedToCopy'] = 'added To Copy';
} catch (Exception $e) {
	echo "Exception:" . $e->getMessage() . "\n";
}
$original->addedToOriginal = 'added To Original';
var_dump($ao, $original, $copy);

echo "\n\n--> exchangeArray() with bad arg type:\n";
unset($original, $ao, $swapIn, $copy);
$original = new C;
$ao = new ArrayObject($original);
try {
	$copy = $ao->exchangeArray(null);
	$copy['addedToCopy'] = 'added To Copy';
} catch (Exception $e) {
	echo "Exception:" . $e->getMessage() . "\n";
}
$original->addedToOriginal = 'added To Original';
var_dump($ao, $original, $copy);

?>