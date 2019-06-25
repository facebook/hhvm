<?hh

class obj extends SplFixedArray{
    public function offsetSet($offset, $value) {
        var_dump($offset);
    }
}
<<__EntryPoint>> function main(): void {
$obj = new obj;

$obj[]=2;
$obj[]=2;
$obj[]=2;
}
