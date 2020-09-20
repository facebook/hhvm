<?hh

class Foo {
    public function __toString() {
        return 'Foo';
    }
}

function test($options, $queryPart) {
    return ''. (0 ? 1 : $queryPart);
}

<<__EntryPoint>> function main(): void {
$options = varray[];

var_dump(test($options, new Foo()));
}
