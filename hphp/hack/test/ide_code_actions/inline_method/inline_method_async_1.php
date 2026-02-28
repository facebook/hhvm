<?hh

class A {
    private async function inline_me(string $param_a): Awaitable<string> {
        $z = $param_a;
        return $z;
    }
    public async function main(): Awaitable<void> {
        echo "test\n";
        $s = await $this->/*range-start*/inline_me/*range-end*/("hello ");
        echo $s . "\n";
    }
}
