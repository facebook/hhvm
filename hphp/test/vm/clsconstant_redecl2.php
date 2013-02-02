<?php

interface A {
  const FOO = 'FOO';
}

interface B extends A {
  const BAR = 'BAR';
}

class C implements B {
  const FOO = 'DOH';
}
