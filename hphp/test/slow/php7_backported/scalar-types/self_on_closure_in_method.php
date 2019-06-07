<?hh

class A {
    public function test() {
        return function() : self {
            return $this;
        };
    }
}
<<__EntryPoint>> function main() {
var_dump(((new A)->test())());
}
