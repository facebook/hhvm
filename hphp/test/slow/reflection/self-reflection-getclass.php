<?hh

class Foo
{
    public function compare(self $otherFoo) { return $otherFoo === $this; }
}


<<__EntryPoint>>
function main_self_reflection_getclass() {
$rc = (new ReflectionParameter(varray['Foo', 'compare'], 'otherFoo'))->getClass();
var_dump(get_class($rc));
var_dump($rc->getName());
}
