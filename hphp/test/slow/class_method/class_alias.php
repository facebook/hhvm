<?php

class A {}

class_alias( 'A', 'B' );

interface UserOfA {
  public function someMethod(B $a);
}

class ActualUserOfA implements UserOfA {
  public function someMethod(A $a) { }
}

//

class A1 {}

class_alias( 'A1', 'B1' );

interface UserOfA1 {
  public function someMethod(A1 $a);
}

class ActualUserOfA1 implements UserOfA1 {
  public function someMethod(B1 $a) { }
}
