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
        forward_static_call('A::test');
    }
}

B::test('foo');
