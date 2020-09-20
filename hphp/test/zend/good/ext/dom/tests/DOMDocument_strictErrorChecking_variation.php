<?hh
<<__EntryPoint>> function main(): void {
echo "Load document\n";
$doc = new DOMDocument;
$doc->load(dirname(__FILE__)."/book.xml");

echo "See if strictErrorChecking is on\n";
var_dump($doc->strictErrorChecking);

echo "Should throw DOMException when strictErrorChecking is on\n";
try {
	$attr = $doc->createAttribute('0');
} catch (DOMException $e) {
	echo "GOOD. DOMException thrown\n";
	echo $e->getMessage() ."\n";
} catch (Exception $e) {
	echo "OOPS. Other exception thrown\n";
}


echo "Turn strictErrorChecking off\n";
$doc->strictErrorChecking = false;

echo "See if strictErrorChecking is off\n";
var_dump($doc->strictErrorChecking);

echo "Should raise PHP error because strictErrorChecking is off\n";
try {
	$attr = $doc->createAttribute('0');
} catch (DOMException $e) {
	echo "OOPS. DOMException thrown\n";
	echo $e->getMessage() ."\n";
} catch (Exception $e) {
	echo "OOPS. Other exception thrown\n";
}
}
