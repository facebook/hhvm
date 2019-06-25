<?hh

class A {
    public function test() {
        return function() : self {
            return $this;
        };
    }
}
<<__EntryPoint>> function main(): void {
var_dump(((new A)->test())());
}
