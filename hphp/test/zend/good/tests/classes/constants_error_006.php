<?php
class aclass
{
    const myConst = "hello";
}
<<__EntryPoint>> function main() {
echo "\nTrying to modify a class constant directly - should be parse error.\n";
aclass::myConst = "no!!";
var_dump(aclass::myConst);
}
