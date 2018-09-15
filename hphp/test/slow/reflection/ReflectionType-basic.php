<?hh

function foo(int $a): ?bool {}


<<__EntryPoint>>
function main_reflection_type_basic() {
$rf = new ReflectionFunction('foo');

echo "--Parameter--\n\n";

$rp = $rf->getParameters()[0];
var_dump($rp->hasType());
$rt = $rp->getType();
var_dump($rt->isBuiltin());
var_dump($rt->__toString());
var_dump($rt->allowsNull());

echo "\n--Return--\n\n";

var_dump($rf->hasReturnType());
$rt = $rf->getReturnType();
var_dump($rt->isBuiltin());
var_dump($rt->__toString());
var_dump($rt->allowsNull());


echo "\n--Call Constructor Directly--\n\n";

// There is a public constructor; we differ with PHP 7 a bit here. In PHP 7
// you can call the public constructor from user code and then you fatal on
// the first method call to the instance. In HHVM, you can call the constructor
// directly, but you have to have an instance of ReflectionParameter or
// ReflectionFunctionAbstract.
$rt2 = new ReflectionType($rp);
// this returns false since we didn't pass any info to the constructor
var_dump($rt2->isBuiltin());

// This will trigger an error since it is not ReflectionParameter or
// ReflectionFunctionAbstract
$rt3 = new ReflectionType(new ReflectionClass('Exception'));
}
