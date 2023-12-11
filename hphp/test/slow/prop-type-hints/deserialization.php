<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class A {
  public int $p1 = 123;
  public string $p2 = 'abc';
  public nonnull $p3 = 456;
  public int $p4 = 789;
  public bool $p5 = true;
  public vec $p6 = vec[1, 2, 3];
  public Traversable $p7;
  public varray $p8 = vec[];
  public darray $p9 = dict[];
  public ?AnyArray $p10;
}

class B extends A {
  public int $p1 = 123;
}
<<__EntryPoint>>
function test() :mixed{
  $s = 'O:1:"B":10:{s:2:"p1";i:123;s:2:"p2";s:3:"abc";s:2:"p3";i:456;s:2:"p4";i:789;s:2:"p5";b:1;s:2:"p6";v:3:{i:1;i:2;i:3;}s:2:"p7";v:0:{}s:2:"p8";y:0:{}s:2:"p9";Y:0:{}s:3:"p10";a:0:{}}';
  unserialize($s);

  $s2 = 'O:1:"B":9:{s:2:"p1";s:3:"zyz";s:2:"p2";i:707;s:2:"p3";N;s:2:"p4";i:789;s:2:"p5";i:1;s:2:"p6";k:3:{i:1;i:2;i:3;}s:2:"p8";Y:0:{}s:2:"p9";a:0:{}s:3:"p10";y:0:{}}';
  unserialize($s2);
}
