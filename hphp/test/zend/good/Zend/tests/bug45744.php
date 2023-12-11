<?hh

class Foo {
    public function __construct(varray $data) {
        var_dump(array_map(vec[$this, 'callback'], $data));
    }

    public function callback($value) :mixed{
        if (!is_array($value)) {
            return stripslashes($value);
        }
    return array_map(vec[$this, 'callback'], $value);
    }
}

class Bar extends Foo {
}

class Foo2 {
    public function __construct(varray $data) {
        var_dump(array_map(vec[$this, 'callBack'], $data));
    }

    public function callBack($value) :mixed{
    }
}

class Bar2 extends Foo2 {
}

<<__EntryPoint>> function main(): void {
new Bar(vec[]);

new Bar2(vec[]);
}
