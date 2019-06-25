<?hh

class foo extends ArrayObject {
    public function __construct($arr) {
        parent::__construct($arr);
    }
}
<<__EntryPoint>> function main(): void {
var_dump( (new foo( array(1, array(4, 5), 3) ))[1][0] ); // int(4)
}
