<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

interface IBox {
};

class Box<T> implements IBox {
    public function __construct(public T $data) {
    }
};

enum class Foo: IBox {
    Box<string> foo = new Box("foo");
};

enum class FooBar: IBox extends Foo {
    Box<string> bar = new Box("bar");
};

<<__EntryPoint>>
function main(): void {
    try {
        $foo_values = Foo::getValues();
        $foobar_values = FooBar::getValues();

        if ($foo_values['foo'] == $foobar_values['foo']) {
            echo "same\n";
        } else {
            echo "not same\n";
        }
        if ($foo_values['foo'] === $foobar_values['foo']) {
            echo "identical\n";
        } else {
            echo "not identical\n";
        }
    } catch (Exception $e) {
        echo 'Exception: '.$e->getMessage()."\n";
    }
}
