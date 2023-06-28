<?hh

trait foo {
    public function __clone() :mixed{
        var_dump(__FUNCTION__);
    }
}

trait baz {
    public function __clone() :mixed{
        var_dump(__FUNCTION__);
    }
}

class bar {
    use foo;
    use baz;
}
<<__EntryPoint>> function main(): void {
$o = new bar;
var_dump(clone $o);
}
