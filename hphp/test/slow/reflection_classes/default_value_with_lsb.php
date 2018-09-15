<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

abstract class JJTest {
  const ANSWER = 42;
  public function test(int $answer = static::ANSWER): void {}
}

echo "hello\n";
$r = new ReflectionClass(JJTest::class);
var_dump($r->getMethod('test')->getParameters());
