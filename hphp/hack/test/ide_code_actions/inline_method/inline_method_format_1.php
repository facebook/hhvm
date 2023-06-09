<?hh

class A {
    private function inline_me(string $param_a, int $param_b): string {
        $z = $param_a;
        if ($param_b > 1) {
            $z = $param_a . $param_b;
        }
        return $z;
    }
    public function main(): void {
        if (true) {
            echo "test\n";
            $s = $this->/*range-start*/inline_me/*range-end*/("hello " . "world", 2 + 2);
            echo $s . "\n";
        }
    }
}
