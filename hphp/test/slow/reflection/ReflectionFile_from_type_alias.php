<?hh

type MyType = Vector<int>;
newtype MyOpaqueType = (function(string, Set<int>): void);
<<__EntryPoint>> function main(): void {
$myType = new ReflectionTypeAlias('MyType');
$fileViaType = $myType->getFile();
var_dump($fileViaType->getName() === __FILE__);
var_dump($fileViaType->getAttributesNamespaced());

$myOpaqueType = new ReflectionTypeAlias('MyOpaqueType');
$fileViaNewtype = $myOpaqueType->getFile();
var_dump($fileViaNewtype->getName() === __FILE__);
var_dump($fileViaNewtype->getAttributesNamespaced());
}
