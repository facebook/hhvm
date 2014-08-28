<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

/**
 * @emails coefficient-dev@lists.facebook.com
 */

function sigmoid(float $x) {
  return 1.0 / (1.0 + exp(- 2 * $x));
}

$class = new BoostingModelNative();

function testModel1() {

  global $class;
  $model =
'(Lf -1.0)|
(Tr 1:10 (Lf 2) (Lf 4))|
(Tr 2:20 (Tr 3:30 (Lf 8) (Lf 16.0)) (Tr 3:300 (Lf 32) (Lf 64.0)))';
  // Features 1, 2, 3
  echo sigmoid(9.0) . "\n";
  echo $class->boostingEvaluate($model, array(0.0, 0.0, 0.0, 0.0))
       . "\n\n";

  echo sigmoid(11.0) . "\n";
  echo $class->boostingEvaluate($model, array(0.0, 20.0, 0.0, 0.0))
       . "\n\n";

  echo sigmoid(19.0) -
      $class->boostingEvaluate($model, array(0.0, 20.0, 0.0, 60.0))
      . "\n\n";

  echo sigmoid(35.0) -
      $class->boostingEvaluate($model, array(0.0, 20.0, 40.0, 60.0))
      . "\n\n";

  echo sigmoid(65.0) -
      $class->boostingEvaluate($model, array(0.0, 0.0, 40.0, 340.0))
      . "\n\n";
}

function testModel2() {

  global $class;
  $model =
'(Tr 1:10 (Lf 1) (Tr 2:20 (Lf 2) (Lf 4))|
(Tr 3:10 (Tr 4:20 (Lf 8) (Tr 4:30 (Lf 16) (Lf 32))) (Lf 64))|
(Lf -1.0)';
  // Features 1, 2, 3, 4, 5
  echo sigmoid(8.0) . "\n";
  echo $class->boostingEvaluate($model, array(0.0, 0.0, 0.0, 0.0, 0.0))
       . "\n\n";

  echo sigmoid(8.0) . "\n";
  echo $class->boostingEvaluate($model, array(0.0, 9.0, 0.0, 9.0, 0.0))
       . "\n\n";

  echo sigmoid(17.0) -
      $class->boostingEvaluate($model, array(0.0, 11.0, 0.0, 9.0, 25.0))
      . "\n\n";

  echo sigmoid(21.0) -
      $class->boostingEvaluate($model, array(0.0, 11.0, 25.0, 9.0, 25.0))
      . "\n\n";

  echo sigmoid(35.0) -
      $class->boostingEvaluate($model, array(0.0, 11.0, 25.0, 9.0, 35.0))
      . "\n\n";

  echo sigmoid(65.0) -
      $class->boostingEvaluate($model, array(0.0, 11.0, 19.0, 11.0, 35.0))
      . "\n\n";
}


testModel1();
echo "---\n";
testModel2();
