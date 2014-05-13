<?php

const ONE = 1;
const TWO = ONE + 1;
const THREE = TWO + ONE;
const FOUR = TWO * TWO;
const FIVE = (FOUR * TWO) - THREE;

class C {
  const STRING = 'I am a string';

  const THREE = TWO + 1;
  const ONE_THIRD = ONE / self::THREE;
  const SENTENCE = 'The value of THREE is '.self::THREE;

  public function f($a = ONE + self::THREE) {
    return $a;
  }
}

class D {
  const NULL = null;
  const THREE_IS_CORRECT = (THREE === C::THREE) ? true : false;
  const CHOOSE = self::THREE_IS_CORRECT ? (ONE / ONE) : (ONE + ONE);
  const PROTECT = self::NULL ?: self::CHOOSE;
}

function main() {
  var_dump(ONE);
  var_dump(TWO);
  var_dump(THREE);
  var_dump(FOUR);
  var_dump(FIVE);

  var_dump((new C)->f());
  var_dump((new C)->f(1));

  var_dump(C::THREE);
  var_dump(C::ONE_THIRD);
  var_dump(C::SENTENCE);

  var_dump(D::THREE_IS_CORRECT);
  var_dump(D::CHOOSE);
  var_dump(D::PROTECT);
}

main();
