<?php

interface A {
  const FOO = 'FOO';
}

class B implements A {
  const FOO = 'BAR';
}
