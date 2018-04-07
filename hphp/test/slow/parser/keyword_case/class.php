<?php

ABSTRACT CLASS A {
  CONST MESSAGE = "I <3 COBOL\n";
  PUBLIC STATIC FUNCTION print_message() {
    ECHO SELF::MESSAGE;
  }
  ABSTRACT PROTECTED FUNCTION charlie();
}

TRAIT T1 {
  PRIVATE FUNCTION able() {
    ECHO "T1 ABLE\n";
  }
  PROTECTED FUNCTION baker() {
    ECHO "T1 BAKER\n";
  }
}

TRAIT T2 {
  PRIVATE FUNCTION able() {
    ECHO "T2 ABLE\n";
  }
}

INTERFACE I {
  PUBLIC FUNCTION test();
}

FINAL CLASS C EXTENDS A IMPLEMENTS I {
  USE T1, T2 {
    T2::able INSTEADOF T1;
    T1::baker AS charlie;
  }

  VAR $x;
  PUBLIC $y;

  FINAL PUBLIC FUNCTION test() {
    SELF::print_message();
    $this->able();
    $this->charlie();
    ECHO "x = {$this->x}; y = {$this->y}\n";
  }
}

$c = NEW C();
$c->x = 1;
$c->y = "two";
$c2 = CLONE $c;
$c2->x = "one";
$c2->y = 2;

var_dump($c INSTANCEOF I);
$c->test();
$c2->test();
