<?hh

class Foo {
    public function __toString() :mixed{
        return 'Foo';
    }
}

function test($options, $queryPart) :mixed{
    return ''. (0 ? 1 : $queryPart);
}

<<__EntryPoint>> function main(): void {
$options = vec[];

var_dump(test($options, new Foo()));
}
