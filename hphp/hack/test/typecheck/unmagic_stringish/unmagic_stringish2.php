<?hh

class CNonStringish {}

class CExplicit implements StringishObject{
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
  require implements StringishObject;

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

function test(): void {
  $cnon = new CNonStringish();

  $cexp = new CExplicit();
  $cimp = new CImplicit();
  $cimpintf = new CImplicitIntf();
  $cimptuse = new CImplicitTuse();
  $cimptreq = new CImplicitTreq();

  "$cnon";
  ''.$cnon;

  "$cexp";
  ''.$cexp;

  "$cimp";
  ''.$cimp;

  "$cimpintf";
  ''.$cimpintf;

  "$cimptuse";
  ''.$cimptuse;

  "$cimptreq";
  ''.$cimptreq;
}
