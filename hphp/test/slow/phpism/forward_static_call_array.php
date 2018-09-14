<?hh
class A
{
    public static function test() {
        var_dump(static::class);
    }
}

class B extends A
{
    public static function test() {
        forward_static_call_array(array('A', 'test'), array('more', 'args'));
    }
}

B::test('foo');
