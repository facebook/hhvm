<?php

trait T
{
  public function __construct()
  {
    echo "Trait contructor\n";
  }
}

class A
{
  public function __construct()
  {
    echo "Parent constructor\n";
  }
}

class B extends A
{
  use T;
}

new B();

