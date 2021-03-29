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

// another child of Foo, but not of FooBar
enum class FooBar2: IBox extends Foo {
    Box<string> bar = new Box("bar");
};

<<__EntryPoint>>
function main(): void {
    try {
        if (FooBar2::foo == FooBar::foo) {
            echo "same\n";
        } else {
            echo "not same\n";
        }
        if (FooBar2::foo === FooBar::foo) {
            echo "identical\n";
        } else {
            echo "not identical\n";
        }

        if (FooBar2::bar == FooBar::bar) {
            echo "same\n";
        } else {
            echo "not same\n";
        }
        if (FooBar2::bar === FooBar::bar) {
            echo "identical\n";
        } else {
            echo "not identical\n";
        }

    } catch (Exception $e) {
        echo 'Exception: '.$e->getMessage()."\n";
    }
}
