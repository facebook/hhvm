<?hh

class StopLintFromFiring{}

abstract class C {
    abstract const type TFoo as num;
    abstract public function get(): this::TFoo;
    public function takesString(string $x): void {}
    public function test(C $c): void {
        // $c->get() : <expr#1>::TFoo
        $this->takesString(HH\FIXME\UNSAFE_CAST<mixed,string>($c->get()));
    }
}
