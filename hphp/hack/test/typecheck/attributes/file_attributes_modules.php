//// file1.php
<?hh // strict

class Ok implements \HH\FileAttribute {
    public function __construct(string $_) {}
}

class Err1 implements \HH\FileAttribute {
    public function __construct(string $_) {}
}

class Err2 implements \HH\FileAttribute {
    public function __construct(string $_) {}
}
//// file2.php
<?hh // strict

<<file:Ok(''), Err1(10), Err2(NonExistent::class)>>

new module example {}
