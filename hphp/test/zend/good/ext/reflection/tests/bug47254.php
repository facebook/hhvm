<?hh
class A
{
    protected function a() :mixed{}

}

class B extends A
{
    public function b() :mixed{}
}
<<__EntryPoint>> function main(): void {
$B = new B();
$R = new ReflectionObject($B);
$m = $R->getMethods();
print_r($m);
}
