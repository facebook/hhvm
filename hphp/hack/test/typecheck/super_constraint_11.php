<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

interface MyWidget<Tu> {}

// Create a pair of unrelated types
class WidgetParent {}
class NotWidgetParent {}

class MyWidgetA implements MyWidget<WidgetParent> {}
class MyWidgetB implements MyWidget<NotWidgetParent> {}

class MyContainer {
  public function add<T super WidgetParent>(MyWidget<T> $task): void {}
}

function test_hhvm_super(): void {
  $container = new MyContainer();
  // So T = WidgetParent
  $container->add(new MyWidgetA()); // this should pass the type checker
  // So T = NotWidgetParent
  $container->add(new MyWidgetB()); // this should be flagged as an error
}
