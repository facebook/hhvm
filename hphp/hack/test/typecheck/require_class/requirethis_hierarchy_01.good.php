<?hh

class C {
    use T2;

    public function bar(): void {}
}

trait T1 {
    require this as C;
}

trait T2 {
    use T1;

    public function foo(): void {
        $this->bar();
    }
}
