<?php
interface foo {
    public function bar() : foo;
}

interface biz {}

class qux implements foo {
    public function bar() : biz {
        return $this;
    }
}

