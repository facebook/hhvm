<?hh // strict

class CExplicit implements Stringish {
  public function __toString(): string {
    return __CLASS__;
  }
}

class CImplicit {
  public function __toString(): string {
    return __CLASS__;
  }
}

interface IImplicit {
  public function __toString(): string;
}

class CImplicitIntf implements IImplicit {
  public function __toString(): string {
    return __CLASS__;
  }
}

trait TStringish {
  public function __toString(): string {
    return __TRAIT__;
  }

  private function foo(): string {
    return (string) $this;
  }
}

class CImplicitTuse {
  use TStringish;
}

trait TReq {
  require implements Stringish;

  private function foo(): string {
    return (string) $this;
  }
}

class CImplicitTreq {
  use TReq;

  public function __toString(): string {
    return __CLASS__;
  }
}

function make_num(): num {
  return 10101;
}

function make_arraykey(): arraykey {
  return '10101';
}

function make_mixed(): mixed {
  return 10101;
}

function make_nonnull(): nonnull {
  return 10101;
}

function test(): void {
  $bool = true;
  $int = 10101;
  $float = 10.101;
  $string = '10101';
  $null = null;
  $num = make_num();
  $arraykey = make_arraykey();

  $mixed = make_mixed();
  $nonnull = make_nonnull();

  $cexp = new CExplicit();
  $cimp = new CImplicit();
  $cimpintf = new CImplicitIntf();
  $cimptuse = new CImplicitTuse();
  $cimptreq = new CImplicitTreq();

  (string) $bool;
  (string) $int;
  (string) $float;
  (string) $string;
  (string) $null;
  (string) $num;
  (string) $arraykey;

  (string) $mixed;
  (string) $nonnull;

  (string) $cexp;
  (string) $cimp;
  (string) $cimpintf;
  (string) $cimptuse;
  (string) $cimptreq;
}
