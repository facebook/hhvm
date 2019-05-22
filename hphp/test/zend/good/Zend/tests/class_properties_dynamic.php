<?php
class Foo {
    const BAR = 1 << 0;
    const BAZ = 1 << 1;
    public $bar = self::BAR | self::BAZ;
}
<<__EntryPoint>> function main() {
echo (new Foo)->bar;
}
