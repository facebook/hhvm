<?hh
class MyObjet implements ArrayAccess
{
    public function offsetSet($offset, $value) { }
    public function offsetExists($offset) {  }
    public function offsetUnset($offset) { }

    public function offsetGet ($offset)
    {
    return function ($var) use ($offset) { // here is the problem
              var_dump($offset, $var);
        };
    }
}
<<__EntryPoint>> function main(): void {
$a = new MyObjet();
echo $a['p']('foo');
}
