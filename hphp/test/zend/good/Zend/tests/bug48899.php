<?hh

class ParentClass { }

class ChildClass extends ParentClass {
    public function testIsCallable() :mixed{
        var_dump(is_callable(varray[$this, 'parent::testIsCallable']));
    }
    public function testIsCallable2() :mixed{
        var_dump(is_callable(varray[$this, 'static::testIsCallable2']));
    }
}
<<__EntryPoint>> function main(): void {
$child = new ChildClass();
$child->testIsCallable();
$child->testIsCallable2();
}
