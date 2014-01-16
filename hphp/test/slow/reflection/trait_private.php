<?php

trait P1 {
  private $p1;
}
trait C1 {
  private $c1;
}

class P2 {
  use P1;
  private $p2;
}
class C2 extends P2 {
  use C1;
  private $c2;
}

$p = new ReflectionClass(new P2);
var_dump($p->hasProperty('p2'));
var_dump($p->hasProperty('p1'));
var_dump($p->getTraits()['P1']->hasProperty('p1'));

$c = new ReflectionClass(new C2);
var_dump($c->hasProperty('p2'));
var_dump($c->hasProperty('p1'));
var_dump(isset($c->getTraits()['P1']));
var_dump($c->getTraits()['C1']->hasProperty('c1'));
