<?php
class foo {
    public function __get($member) {
        $f = array("foo"=>"bar","bar"=>"foo");
        return $f;
    }
}
<<__EntryPoint>> function main() {
$f = new foo();
foreach($f->bar as $key => $value) {
    print "$key => $value\n";
}
}
