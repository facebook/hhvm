<?hh // strict
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *
 */

class Node<T> {
  public ?Node<T> $left, $right;
  public T $data;

  public function __construct(T $data, ?Node<T> $left, ?Node<T> $right) {
    $this->left = $left;
    $this->right = $right;
    $this->data = $data;
  }
}

function make_tree(int $n): ?Node<int> {
  if($n <= 0) {
    return null;
  }
  else {
    return new Node($n, make_tree($n-1), make_tree($n-2));
  }
}

function gen(?Node<int> $tree): Generator<int, int, void> {

  if ($tree === null) {

    yield 0;

  }
  else {
    yield $tree->data;

    $left = gen($tree->left);

    foreach($left as $v) {
      yield $v;
    }

    $right = gen($tree->right);

    foreach($right as $v) {
      yield $v;
    }
  }
}

function nat(): Generator<int, int, void> {
  for($i = 0; $i < 10000; $i++) {
    yield $i;
  }
}

function testArray(): void {
  $left = nat();
  $x = Vector {};
  $left->rewind();
  $acc = 0;
  while($left->valid()) {
    $left->next();
    $x[] = $left->current();
  }
  foreach($x as $v) {
    $acc += $v;
  }
}

function gen2(int $n): Generator<int, int, void> {
  if($n <= 0) {
    yield 0;
    yield 1;
    yield 2;
  }
  else {
    $x = gen2($n - 1);
    foreach($x as $v) {
      yield $v;
    }
  }
}

function main(): void {
  testArray();

  $t = make_tree(5);
  if($t === null) { return; }

  $x = gen($t);
  $acc = 0;
  $x->rewind();
  while($x->valid()) {
    $x->next();
    $v = $x->current();
    $acc += $v;
  }
  if($acc === 26) {
    echo 'OK';
  }
  else {
    echo 'Failure: test_yield_erling.1';
  }


  $x = gen2(5);
  $acc = 0;
  foreach($x as $v) {
    $acc += $v;
  }
  if($acc === 3) {
    echo 'OK';
  }
  else {
    echo 'Failure: test_yield_erling.1';
  }

}
