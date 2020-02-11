<?hh
class MyContainer
{
    public function __get( $what )
    {
        return $this->_p[ $what ];
    }

    public function __set( $what, $value )
    {
        $this->_p[ $what ] = $value;
    }

    private $_p = varray[];
}
<<__EntryPoint>> function main(): void {
$c = new MyContainer();
$c->a = 1;
$c->a += 1;
print $c->a;    // --> 2

print " - ";
$c->a += max( 0, 1 );
print $c->a;    // --> 4 (!)
}
