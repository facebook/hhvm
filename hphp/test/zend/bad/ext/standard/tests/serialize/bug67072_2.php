<?php
class MySplFileObject extends SplFileObject {}
class MyArrayObject extends ArrayObject{ var $a = 1; }
echo unserialize('O:15:"MySplFileObject":1:{s:9:"*filename";s:15:"/home/flag/flag";}');

function testClass($className) 
{
    // simulate phpunit
    $object = unserialize(sprintf('O:%d:"%s":0:{}', strlen($className), $className));
    return $object;
}

class MyClass {}
class MyClassSer implements Serializable {
        function serialize() { return "";}
        function unserialize($data) { }
}
class MyClassSer2 extends MyClassSer {
}

$classes = array('stdClass', 'MyClass', 'MyClassSer', 'MyClassSer2', 'SplFileObject', 'MySplFileObject', 
                 'SplObjectStorage', 'FooBar', 'Closure', 'ArrayObject', 'MyArrayObject',
                 'Directory'
             );
foreach($classes as $cl) {
        var_dump(testClass($cl));
}

?>
===DONE==
