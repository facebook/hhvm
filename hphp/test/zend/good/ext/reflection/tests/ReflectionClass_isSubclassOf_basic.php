<?hh
class A {}
class B extends A {}
class C extends B {}

interface I {}
class X implements I {}
<<__EntryPoint>> function main(): void {
$classNames = vec['A', 'B', 'C', 'I', 'X'];
$rcs = dict[];
foreach ($classNames as $className) {
    $rcs[$className] = new ReflectionClass($className);
}

foreach ($rcs as $childName => $child) {
    foreach ($rcs as $parentName => $parent) {
        echo "Is " . $childName . " a subclass of " . $parentName . "? \n";
        echo "   - Using object argument: ";
        var_dump($child->isSubclassOf($parent));
        echo "   - Using string argument: ";
        var_dump($child->isSubclassOf($parentName));
    }
}
}
