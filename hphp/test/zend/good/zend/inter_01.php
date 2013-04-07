<?php
interface foo {
    const foo = 'foobar';
    public function bar($x = foo);
}

class foobar implements foo {
    const foo = 'bar';
    public function bar($x = foo::foo) {
        var_dump($x);
    }    
}
?>