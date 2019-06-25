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
$options = array();

var_dump(test($options, new Foo()));
}
