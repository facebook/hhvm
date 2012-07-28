<?php

// class with __get and __set
class A {
  private $x;
  public $y;

  function unsetall() {
    unset($this->x);
    unset($this->y);
  }

  function __get($n) {
    echo "A::__get $n\n";
    return $n;
  }

  function __set($n, $v) {
    echo "A::__set $n, $v\n";
  }

  function setprop() {
    $this->x = 1;
    $this->y = 2;
    var_dump($this);
  }

  function setopprop() {
    $this->x += 1;
    $this->y += 2;
    var_dump($this);
  }

  function incdecprop() {
    $this->x++;
    $this->y++;
    var_dump($this);
  }
}

// class with __get but not __set
class B {
  private $x;
  public $y;

  function unsetall() {
    unset($this->x);
    unset($this->y);
  }

  function __get($n) {
    echo "B::__get $n\n";
    return $n;
  }

  function setprop() {
    $this->x = 1;
    $this->y = 2;
    var_dump($this);
  }

  function setopprop() {
    $this->x += 1;
    $this->y += 2;
    var_dump($this);
  }

  function incdecprop() {
    $this->x++;
    $this->y++;
    var_dump($this);
  }
}

// class with __set but not __get
class C {
  private $x;
  public $y;

  function unsetall() {
    unset($this->x);
    unset($this->y);
  }

  function __set($n, $v) {
    echo "C::__set $n, $v\n";
  }

  function setprop() {
    $this->x = 1;
    $this->y = 2;
    var_dump($this);
  }

  function setopprop() {
    $this->x += 1;
    $this->y += 2;
    var_dump($this);
  }

  function incdecprop() {
    $this->x++;
    $this->y++;
    var_dump($this);
  }
}

// class without __get or __set
class D {
  private $x;
  public $y;

  function unsetall() {
    unset($this->x);
    unset($this->y);
  }

  function setprop() {
    $this->x = 1;
    $this->y = 2;
    var_dump($this);
  }

  function setopprop() {
    $this->x += 1;
    $this->y += 2;
    var_dump($this);
  }

  function incdecprop() {
    $this->x++;
    $this->y++;
    var_dump($this);
  }
}

function propd(&$x) {
  var_dump($x);
}

$a = new A;
// unset all properties
$a->unsetall();
// Prop for visible, accessible property: use __get
var_dump($a->y);
// Prop for visible, inaccessible property: use __get
var_dump($a->x);
// PropD for visible, accessible property: use __get
propd($a->y);
// PropD for visible, inaccessible property: use __get
propd($a->x);
// PropU for visible, accessible property
unset($a->y);
/*
// SetProp for visible, accessible properties: use __set
$a->setprop();
*/
// SetOpProp: use __set and __get
$a->unsetall();
$a->setopprop();
// IncDecProp: use __set and __get
$a->unsetall();
$a->incdecprop();

$b = new B;
// unset all properties
$b->unsetall();
// SetOpProp: use __get
$b->setopprop();
// IncDecProp: use __get
$b->unsetall();
$b->incdecprop();

$c = new C;
// unset all properties
$c->unsetall();
// SetOpProp
$c->setopprop();
// IncDecProp
$c->unsetall();
$c->incdecprop();

$d = new D;
// unset all properties
$d->unsetall();
// Prop for visible, accessible property
var_dump($d->y);
// PropD for visible, accessible property
propd($d->y);
// PropU for visible, accessible property
unset($d->y);
// SetProp for visible, accessible properties
$d->setprop();
// SetOpProp
$d->unsetall();
$d->setopprop();
// IncDecProp
$d->unsetall();
$d->incdecprop();
