<?php

print "Test begin\n";

class C {
  public $preInc = 0;
  public $preDec = 0;
  public $postInc = 0;
  public $postDec = 0;
}

class D {
  private $container = array("a" => "D::a",
                             "b" => 42,
                             "l" => 0,
                             "m" => 0,
                             "n" => 0,
                             "o" => 0);
  public $preInc = 0;
  public $preDec = 0;
  public $postInc = 0;
  public $postDec = 0;
  public function __get($k) {
    print "In D::__get($k)\n";
    return isset($this->container[$k]) ? $this->container[$k] : null;
  }
}

class E {
  private $container = array("a" => "E::a",
                             "b" => 42,
                             "l" => 0,
                             "m" => 0,
                             "n" => 0,
                             "o" => 0);
  public $preInc = 0;
  public $preDec = 0;
  public $postInc = 0;
  public $postDec = 0;
  public function __get($k) {
    print "In E::__get($k)\n";
    return isset($this->container[$k]) ? $this->container[$k] : null;
  }
  public function __set($k, $v) {
    print "In E::__set($k, $v)\n";
    $this->container[$k] = $v;
  }
}

print "--- C ---\n";
$o = new C;
var_dump(++$o->preInc);
var_dump(--$o->preDec);
var_dump($o->postInc++);
var_dump($o->postDec--);
var_dump(++$o->p);
var_dump(--$o->q);
var_dump($o->r++);
var_dump($o->s--);
print_r($o);

print "--- D ---\n";
$o = new D;
var_dump(++$o->a);
var_dump(++$o->b);
var_dump(++$o->preInc);
var_dump(--$o->preDec);
var_dump($o->postInc++);
var_dump($o->postDec--);
var_dump(++$o->l);
var_dump(--$o->m);
var_dump($o->n++);
var_dump($o->o--);
var_dump(++$o->p);
var_dump(--$o->q);
var_dump($o->r++);
var_dump($o->s--);
print_r($o);

print "--- E ---\n";
$o = new E;
var_dump(++$o->preInc);
var_dump(--$o->preDec);
var_dump($o->postInc++);
var_dump($o->postDec--);
var_dump(++$o->l);
var_dump(--$o->m);
var_dump($o->n++);
var_dump($o->o--);
var_dump(++$o->p);
var_dump(--$o->q);
var_dump($o->r++);
var_dump($o->s--);
print_r($o);

print "--- null ---\n";
$o = null;
var_dump(++$o->preInc);
var_dump(--$o->preDec);
var_dump($o->postInc++);
var_dump($o->postDec--);
print_r($o);

print "--- 42 ---\n";
$o = 42;
var_dump(++$o->preInc);
print_r($o);
print "\n";

print "Test end\n";
