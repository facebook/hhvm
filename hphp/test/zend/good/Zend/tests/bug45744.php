<?hh

class Foo {
    public function __construct(array $data) {
        var_dump(array_map(varray[$this, 'callback'], $data));
    }

    public function callback($value) {
        if (!is_array($value)) {
            return stripslashes($value);
        }
    return array_map(varray[$this, 'callback'], $value);
    }
}

class Bar extends Foo {
}

class Foo2 {
    public function __construct(array $data) {
        var_dump(array_map(varray[$this, 'callBack'], $data));
    }

    public function callBack($value) {
    }
}

class Bar2 extends Foo2 {
}

<<__EntryPoint>> function main(): void {
new Bar(varray[]);

new Bar2(varray[]);
}
