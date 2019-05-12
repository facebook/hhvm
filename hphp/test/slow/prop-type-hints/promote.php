<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class A {
  public ?int $a1 = null;
  public ?int $a2 = null;
  public ?int $a3 = null;

  public string $b1 = '';
  public string $b2 = '';
  public string $b3 = '';

  public bool $c1 = false;
  public bool $c2 = false;
  public bool $c3 = false;

  public ?arraylike $d1 = null;
  public ?arraylike $d2 = null;
  public ?arraylike $d3 = null;

  public ?stdclass $e1 = null;
  public ?stdclass $e2 = null;
  public ?stdclass $e3 = null;

  public static ?int $f1 = null;
  public static ?int $f2 = null;
  public static ?int $f3 = null;

  public static string $g1 = '';
  public static string $g2 = '';
  public static string $g3 = '';

  public static bool $h1 = false;
  public static bool $h2 = false;
  public static bool $h3 = false;

  public static ?arraylike $i1 = null;
  public static ?arraylike $i2 = null;
  public static ?arraylike $i3 = null;

  public static ?stdclass $j1 = null;
  public static ?stdclass $j2 = null;
  public static ?stdclass $j3 = null;
}

class B extends A {

  public function testObj() {
    $this->a1->x = 123;
    $this->a2->x++;
    $this->a3->x += 123;

    $this->b1->x = 123;
    $this->b2->x++;
    $this->b3->x += 123;

    $this->c1->x = 123;
    $this->c2->x++;
    $this->c3->x += 123;

    $this->d1->x = 123;
    $this->d2->x++;
    $this->d3->x += 123;

    $this->e1->x = 123;
    $this->e2->x++;
    $this->e3->x += 123;

    self::$f1->x = 123;
    self::$f2->x++;
    self::$f3->x += 123;

    self::$g1->x = 123;
    self::$g2->x++;
    self::$g3->x += 123;

    self::$h1->x = 123;
    self::$h2->x++;
    self::$h3->x += 123;

    self::$i1->x = 123;
    self::$i2->x++;
    self::$i3->x += 123;

    self::$j1->x = 123;
    self::$j2->x++;
    self::$j3->x += 123;
  }
}
<<__EntryPoint>> function main(): void {
(new B())->testObj();
}
