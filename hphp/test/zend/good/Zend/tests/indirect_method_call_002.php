<?hh

class foo {
    public $x = 'testing';

    public function bar() :mixed{
        return "foo";
    }
    public function baz() :mixed{
        return new self;
    }
}
<<__EntryPoint>> function main(): void {
var_dump((new foo())->bar());               // string(3) "foo"
var_dump((new foo())->baz()->x);            // string(7) "testing"
var_dump((new foo())->baz()->baz()->bar()); // string(3) "foo"
(new foo())->www();
}
