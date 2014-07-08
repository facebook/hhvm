<?php

interface IHerp {
  const FOO = 'bar';
}

class Derp implements IHerp {
  const FOO = 'baz';
}

var_dump('All classes declared');
