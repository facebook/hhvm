<?hh
class A
{
    protected function a() {}

}

class B extends A
{
    public function b() {}
}
<<__EntryPoint>> function main(): void {
$B = new B();
$R = new ReflectionObject($B);
$m = $R->getMethods();
print_r($m);
}
