<?hh

class C extends ArrayObject {
    public $p = 'object property';
}
<<__EntryPoint>> function main(): void {
$ao = new C(array('p'=>'array element'));
$ao->setFlags(ArrayObject::ARRAY_AS_PROPS);

echo "\n--> Access the real property:\n";
var_dump(isset($ao->p));
var_dump($ao->p);

echo "\n--> Remove the real property and access the array element:\n";
unset($ao->p);
var_dump(isset($ao->p));
var_dump($ao->p);

echo "\n--> Remove the array element and try access again:\n";
unset($ao->p);
var_dump(isset($ao->p));
try { var_dump($ao->p); } catch (Exception $e) { echo $e->getMessage()."\n"; }

echo "\n--> Re-add the real property:\n";
$ao->p = 'object property';
var_dump(isset($ao->p));
var_dump($ao->p);
}
