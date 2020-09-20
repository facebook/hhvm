<?hh
class Callback {
    protected $val = 'hello, world';

    public function __invoke() {
        return $this->val;
    }
}
<<__EntryPoint>> function main(): void {
$cb = new Callback();
echo $cb(),"\n";
$a = varray[1, 2];
$b = array_map($cb, $a);
print_r($b);
}
