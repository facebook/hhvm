<?hh

interface One {
    public function a():mixed;
    public function b():mixed;
    public function c():mixed;
    public function d():mixed;
}

interface Two extends One {
    public function a() : stdClass;
    public function c() : callable;
    public function b() : AnyArray;
    public function d() : int;
}
<<__EntryPoint>> function main(): void {
echo "Done\n";
}
