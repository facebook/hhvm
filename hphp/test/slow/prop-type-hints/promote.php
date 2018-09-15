<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class A {
  public ?int $a1 = null;
  public ?int $a2 = null;
  public ?int $a3 = null;
  public ?int $a4 = null;
  public ?int $a5 = null;
  public ?int $a6 = null;
  public ?int $a7 = null;
  public ?int $a8 = null;

  public string $b1 = '';
  public string $b2 = '';
  public string $b3 = '';
  public string $b4 = '';
  public string $b5 = '';
  public string $b6 = '';
  public string $b7 = '';
  public string $b8 = '';

  public bool $c1 = false;
  public bool $c2 = false;
  public bool $c3 = false;
  public bool $c4 = false;
  public bool $c5 = false;
  public bool $c6 = false;
  public bool $c7 = false;
  public bool $c8 = false;

  public ?arraylike $d1 = null;
  public ?arraylike $d2 = null;
  public ?arraylike $d3 = null;
  public ?arraylike $d4 = null;
  public ?arraylike $d5 = null;
  public ?arraylike $d6 = null;
  public ?arraylike $d7 = null;
  public ?arraylike $d8 = null;

  public ?stdclass $e1 = null;
  public ?stdclass $e2 = null;
  public ?stdclass $e3 = null;
  public ?stdclass $e4 = null;

  public static ?int $f1 = null;
  public static ?int $f2 = null;
  public static ?int $f3 = null;
  public static ?int $f4 = null;
  public static ?int $f5 = null;
  public static ?int $f6 = null;
  public static ?int $f7 = null;
  public static ?int $f8 = null;

  public static string $g1 = '';
  public static string $g2 = '';
  public static string $g3 = '';
  public static string $g4 = '';
  public static string $g5 = '';
  public static string $g6 = '';
  public static string $g7 = '';
  public static string $g8 = '';

  public static bool $h1 = false;
  public static bool $h2 = false;
  public static bool $h3 = false;
  public static bool $h4 = false;
  public static bool $h5 = false;
  public static bool $h6 = false;
  public static bool $h7 = false;
  public static bool $h8 = false;

  public static ?arraylike $i1 = null;
  public static ?arraylike $i2 = null;
  public static ?arraylike $i3 = null;
  public static ?arraylike $i4 = null;
  public static ?arraylike $i5 = null;
  public static ?arraylike $i6 = null;
  public static ?arraylike $i7 = null;
  public static ?arraylike $i8 = null;

  public static ?stdclass $j1 = null;
  public static ?stdclass $j2 = null;
  public static ?stdclass $j3 = null;
  public static ?stdclass $j4 = null;

  public static ?int $k1 = null;
  public static ?int $k2 = null;
  public static ?int $k3 = null;
  public static ?int $k4 = null;
  public static ?int $k5 = null;
  public static ?int $k6 = null;
  public static ?int $k7 = null;
  public static ?int $k8 = null;

  public static string $l1 = '';
  public static string $l2 = '';
  public static string $l3 = '';
  public static string $l4 = '';
  public static string $l5 = '';
  public static string $l6 = '';
  public static string $l7 = '';
  public static string $l8 = '';

  public static bool $m1 = false;
  public static bool $m2 = false;
  public static bool $m3 = false;
  public static bool $m4 = false;
  public static bool $m5 = false;
  public static bool $m6 = false;
  public static bool $m7 = false;
  public static bool $m8 = false;

  public static ?arraylike $n1 = null;
  public static ?arraylike $n2 = null;
  public static ?arraylike $n3 = null;
  public static ?arraylike $n4 = null;
  public static ?arraylike $n5 = null;
  public static ?arraylike $n6 = null;
  public static ?arraylike $n7 = null;
  public static ?arraylike $n8 = null;
}

class B extends A {
  public function testArray() {
    $this->a1[123] = 'abc';
    $this->a2[] = 'abc';
    $this->a3[123]++;
    $this->a4[123] += 123;
    $this->a5[]++;
    $this->a6[] += 123;
    $this->a7[123][123] = 'abc';
    $this->a8[][123] = 'abc';

    $this->b1[123] = 'abc';
    $this->b2[] = 'abc';
    $this->b3[123]++;
    $this->b4[123] += 123;
    $this->b5[]++;
    $this->b6[] += 123;
    $this->b7[123][123] = 'abc';
    $this->b8[][123] = 'abc';

    $this->c1[123] = 'abc';
    $this->c2[] = 'abc';
    $this->c3[123]++;
    $this->c4[123] += 123;
    $this->c5[]++;
    $this->c6[] += 123;
    $this->c7[123][123] = 'abc';
    $this->c8[][123] = 'abc';

    $this->d1[123] = 'abc';
    $this->d2[] = 'abc';
    $this->d3[123]++;
    $this->d4[123] += 123;
    $this->d5[]++;
    $this->d6[] += 123;
    $this->d7[123][123] = 'abc';
    $this->d8[][123] = 'abc';

    self::$k1[123] = 'abc';
    self::$k2[] = 'abc';
    self::$k3[123]++;
    self::$k4[123] += 123;
    self::$k5[]++;
    self::$k6[] += 123;
    self::$k7[123][123] = 'abc';
    self::$k8[][123] = 'abc';

    self::$l1[123] = 'abc';
    self::$l2[] = 'abc';
    self::$l3[123]++;
    self::$l4[123] += 123;
    self::$l5[]++;
    self::$l6[] += 123;
    self::$l7[123][123] = 'abc';
    self::$l8[][123] = 'abc';

    self::$m1[123] = 'abc';
    self::$m2[] = 'abc';
    self::$m3[123]++;
    self::$m4[123] += 123;
    self::$m5[]++;
    self::$m6[] += 123;
    self::$m7[123][123] = 'abc';
    self::$m8[][123] = 'abc';

    self::$n1[123] = 'abc';
    self::$n2[] = 'abc';
    self::$n3[123]++;
    self::$n4[123] += 123;
    self::$n5[]++;
    self::$n6[] += 123;
    self::$n7[123][123] = 'abc';
    self::$n8[][123] = 'abc';
}

  public function testObj() {
    $this->a1->x = 123;
    $this->a2->x++;
    $this->a3->x += 123;
    $this->a4->x[123] = 'abc';

    $this->b1->x = 123;
    $this->b2->x++;
    $this->b3->x += 123;
    $this->b4->x[123] = 'abc';

    $this->c1->x = 123;
    $this->c2->x++;
    $this->c3->x += 123;
    $this->c4->x[123] = 'abc';

    $this->d1->x = 123;
    $this->d2->x++;
    $this->d3->x += 123;
    $this->d4->x[123] = 'abc';

    $this->e1->x = 123;
    $this->e2->x++;
    $this->e3->x += 123;
    $this->e4->x[123] = 'abc';

    self::$f1->x = 123;
    self::$f2->x++;
    self::$f3->x += 123;
    self::$f4->x[123] = 'abc';

    self::$g1->x = 123;
    self::$g2->x++;
    self::$g3->x += 123;
    self::$g4->x[123] = 'abc';

    self::$h1->x = 123;
    self::$h2->x++;
    self::$h3->x += 123;
    self::$h4->x[123] = 'abc';

    self::$i1->x = 123;
    self::$i2->x++;
    self::$i3->x += 123;
    self::$i4->x[123] = 'abc';

    self::$j1->x = 123;
    self::$j2->x++;
    self::$j3->x += 123;
    self::$j4->x[123] = 'abc';
  }
}

(new B())->testArray();
(new B())->testObj();
