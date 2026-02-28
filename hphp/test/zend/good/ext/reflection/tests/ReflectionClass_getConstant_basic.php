<?hh
class C {
    const a = 'hello from C';
}
class D extends C {
}
class E extends D {
}
class F extends E {
    const a = 'hello from F';
}
class X {
}
<<__EntryPoint>> function main(): void {
$classes = vec["C", "D", "E", "F", "X"];
foreach($classes as $class) {
    echo "Reflecting on class $class: \n";
    $rc = new ReflectionClass($class);
    var_dump($rc->getConstant('a'));
    var_dump($rc->getConstant('doesntexist'));
}
}
