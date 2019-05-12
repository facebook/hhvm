<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

type VAlias = varray;
type DAlias = darray;
type DVAlias = varray_or_darray;
type AAlias = array;

class A {
  public varray $v1;
  public varray $v2;
  public varray $v3;
  public ?varray $v4;
  public ?varray $v5;
  public ?varray $v6;

  public darray $d1;
  public darray $d2;
  public darray $d3;
  public ?darray $d4;
  public ?darray $d5;
  public ?darray $d6;

  public varray_or_darray $dv1;
  public varray_or_darray $dv2;
  public varray_or_darray $dv3;
  public ?varray_or_darray $dv4;
  public ?varray_or_darray $dv5;
  public ?varray_or_darray $dv6;

  public array $a1;
  public array $a2;
  public array $a3;
  public ?array $a4;
  public ?array $a5;
  public ?array $a6;

  public VAlias $v1a;
  public DAlias $d1a;
  public DVAlias $dv1a;
  public AAlias $a1a;

  public VAlias $v2a;
  public DAlias $d2a;
  public DVAlias $dv2a;
  public AAlias $a2a;
}

class B extends A {
}

class C extends B {
  public darray $v1;
  public varray_or_darray $v2;
  public array $v3;
  public ?darray $v4;
  public ?varray_or_darray $v5;
  public ?array $v6;

  public varray $d1;
  public varray_or_darray $d2;
  public array $d3;
  public ?varray $d4;
  public ?varray_or_darray $d5;
  public ?array $d6;

  public varray $dv1;
  public darray $dv2;
  public array $dv3;
  public ?varray $dv4;
  public ?darray $dv5;
  public ?array $dv6;

  public varray $a1;
  public darray $a2;
  public varray_or_darray $a3;
  public ?varray $a4;
  public ?darray $a5;
  public ?varray_or_darray $a6;

  public darray $v1a;
  public varray $d1a;
  public array $dv1a;
  public varray_or_darray $a1a;

  public DAlias $v2a;
  public VAlias $d2a;
  public AAlias $dv2a;
  public DVAlias $a2a;

  public static function test() {
    new A();
    new B();
    new C();
  }
}
<<__EntryPoint>> function main(): void {
C::test();
C::test();
echo "DONE\n";
}
