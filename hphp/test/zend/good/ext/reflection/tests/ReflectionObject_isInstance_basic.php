<?hh
class A {}
class B extends A {}
class X {}
<<__EntryPoint>> function main(): void {
$classes = varray["A", "B", "X"];

$instances = darray[    "myA" => new A,
                    "myB" => new B,
                    "myX" => new X ];

foreach ($classes as $class) {
    $ro = new ReflectionObject(new $class);
    foreach ($instances as $name => $instance) {
        echo "is $name a $class? ";
        var_dump($ro->isInstance($instance));
    }
}
}
