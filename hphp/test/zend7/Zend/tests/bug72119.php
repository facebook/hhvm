<?php
interface Foo {
    public function bar(array $baz = null);
}

class Hello implements Foo {
    public function bar(array $baz = [])
    {

    }
}
echo "OK\n";
?>
