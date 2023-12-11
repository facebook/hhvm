<?hh
<<__EntryPoint>> function main(): void {
$obj = new stdClass();
$obj->prop1 = '1';
$obj->prop2 = '2';
$obj->prop3 = '3';

$reflect = new ReflectionObject($obj);

$array = dict[];
foreach($reflect->getProperties() as $prop)
{
	$array[$prop->getName()] = $prop->getValue($obj);
}

var_dump($array);

echo "Done\n";
}
