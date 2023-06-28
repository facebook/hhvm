<?hh
class D {
    // empty class
}

class A extends D {
    // empty class
}


<<__EntryPoint>>
function main_subclassof() :mixed{
$reflection = new \ReflectionClass( '\A' );

var_dump(
    $reflection->isAbstract(),
    $reflection->name,
    $reflection->isSubclassOf( '\D' )
);
}
