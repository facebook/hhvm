<?php
ini_set('error_reporting ',  E_ALL & ~E_DEPRECATED);


class Foo
{
        public function test()
        {
                print 'test';
                throw new Exception();
        }
}

try {
        $bar = new Foo();
        call_user_method_array('test', $bar, array()) ;
} catch (Exception $e) {
}
?>