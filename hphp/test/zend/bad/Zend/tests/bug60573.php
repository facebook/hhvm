<?php
class Foo1 {

public function setSelf(self $s) { }

}

class Bar1 extends Foo1 {

public function setSelf(parent $s) { }

}

class Foo2 {

public function setSelf(Foo2 $s) { }

}

class Bar2 extends Foo2 {

public function setSelf(parent $s) { }

}

class Base {
}

class Foo3 extends Base{

public function setSelf(parent $s) { }

}

class Bar3 extends Foo3 {

public function setSelf(Base $s) { }

}

class Foo4 {

public function setSelf(self $s) { }

}

class Bar4 extends Foo4 {

public function setSelf(self $s) { }

}

class Foo5 extends Base {

public function setSelf(parent $s) { }

}

class Bar5 extends Foo5 {

public function setSelf(parent $s) { }

}

abstract class Foo6 extends Base {

abstract public function setSelf(parent $s);

}

class Bar6 extends Foo6 {

public function setSelf(Foo6 $s) { }

}