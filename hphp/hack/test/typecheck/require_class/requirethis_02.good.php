<?hh

<<file:__EnableUnstableFeatures('require_constraint')>>

class C {
    public function bar(): void { echo "bar\n"; }
}

class D extends C {
    use T;
}

trait T {
    /* HH_IGNORE[12043] require-this-as constraint; class does not use trait */
    require this as C;

    public function foo(): void {
        $this->bar();
    }
}
