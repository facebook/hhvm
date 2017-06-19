<?php

function earlier(){
  function later(){
      class BadClass{ // error2039
      }
      interface BadInterface{ // error2039
      }
      trait BadTrait{ // error2039
      }
    }
}

class ThisIsFine{ // legal
}
interface ThisIsFine{ // legal
}
trait ThisIsFine{ // legal
}

class C {
  public function foo() {
    class D {} // error2039 triggered from inside methods too
  }
}
