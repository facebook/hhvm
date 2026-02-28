<?hh

namespace Main\Test;

class MyClass {
}

function testClass($i, $className) :mixed{
    \printf("%s\n", \class_exists($className) ? 'YES' : 'NO');
    \printf("%s\n", \is_a($i, $className) ? 'YES' : 'NO');
}


<<__EntryPoint>>
function main_instanceof() :mixed{
$i = new MyClass;
$badName = '\Main\Test\MyClass';
$goodName = 'Main\Test\MyClass';

testClass($i, $badName);
testClass($i, $goodName);
}
