<?hh // strict

trait R {
  use T;

  <<Policied>>
  public int $rPub = 0;

  <<Policied>>
  private int $rPri = 0;

  <<Policied>>
  protected int $rPro = 0;

  public int $rUnpolicied = 0;
}

class C extends B {
  use T;

  <<Policied>>
  public int $cPub = 0;

  <<Policied>>
  private int $cPri = 0;

  <<Policied>>
  protected int $cPro = 0;

  public int $cUnpolicied = 0;
}

class A {
  <<Policied>>
  public int $aPub = 0;

  <<Policied>>
  private int $aPri = 0;

  <<Policied>>
  protected int $aPro = 0;

  public int $aUnpolicied = 0;
}

class B extends A {
  <<Policied>>
  public int $bPub = 0;

  <<Policied>>
  private int $bPri = 0;

  <<Policied>>
  protected int $bPro = 0;

  public int $bUnpolicied = 0;
}

trait T {
  <<Policied>>
  public int $tPub = 0;

  <<Policied>>
  private int $tPri = 0;

  <<Policied>>
  protected int $tPro = 0;

  public int $tUnpolicied = 0;
}

trait ZTrait {
  <<Policied>>
  public int $replicaPP = 0;

  <<Policied>>
  public int $replicaPU = 0;

  public int $replicaUP = 0;
}

class Z {
  use ZTrait;

  <<Policied>>
  public int $replicaPP = 0;

  public int $replicaPU = 0;

  <<Policied>>
  public int $replicaUP = 0;
}
