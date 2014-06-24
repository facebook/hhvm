<?php

interface IA {}
interface IB extends IA {}

trait TB {
  require implements IB;
}

class C implements IB {
  use TB;
}

echo 'Done', "\n";
