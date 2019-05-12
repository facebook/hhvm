<?php
class A {
    static function foo() { return 'foo'; }
}
<<__EntryPoint>> function main() {
$classname       =  'A';
$wrongClassname  =  'B';

echo $classname::foo()."\n";
echo $wrongClassname::foo()."\n";
echo "===DONE===\n";
}
