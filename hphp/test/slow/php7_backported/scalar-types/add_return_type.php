<?hh

interface One {
    public function a();
    public function b();
    public function c();
    public function d();
}

interface Two extends One {
    public function a() : stdClass;
    public function c() : callable;
    public function b() : arraylike;
    public function d() : int;
}
<<__EntryPoint>> function main(): void {
echo "Done\n";
}
