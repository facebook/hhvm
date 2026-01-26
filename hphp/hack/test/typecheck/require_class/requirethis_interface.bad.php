<?hh

<<file:__EnableUnstableFeatures('require_constraint')>>

interface I {
    use T;

    public function bar(): void;
}

trait T {
    require this as I;

    public function foo(): void {
        $this->bar();
    }
}
