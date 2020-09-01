<?hh // strict

trait R {
  use T;

  <<Policied("PUBLIC")>>
  public int $rPub = 0;

  <<Policied("PUBLIC")>>
  private int $rPri = 0;

  <<Policied("PUBLIC")>>
  protected int $rPro = 0;

  public int $rUnpolicied = 0;
}

class C extends B {
  use T;

  <<Policied("PUBLIC")>>
  public int $cPub = 0;

  <<Policied("PUBLIC")>>
  private int $cPri = 0;

  <<Policied("PUBLIC")>>
  protected int $cPro = 0;

  public int $cUnpolicied = 0;
}

class A {
  <<Policied("PUBLIC")>>
  public int $aPub = 0;

  <<Policied("PUBLIC")>>
  private int $aPri = 0;

  <<Policied("PUBLIC")>>
  protected int $aPro = 0;

  public int $aUnpolicied = 0;
}

class B extends A {
  <<Policied("PUBLIC")>>
  public int $bPub = 0;

  <<Policied("PUBLIC")>>
  private int $bPri = 0;

  <<Policied("PUBLIC")>>
  protected int $bPro = 0;

  public int $bUnpolicied = 0;
}

trait T {
  <<Policied("PUBLIC")>>
  public int $tPub = 0;

  <<Policied("PUBLIC")>>
  private int $tPri = 0;

  <<Policied("PUBLIC")>>
  protected int $tPro = 0;

  public int $tUnpolicied = 0;
}

trait ZTrait {
  <<Policied("PUBLIC")>>
  public int $replicaPP = 0;

  <<Policied("PUBLIC")>>
  public int $replicaPU = 0;

  public int $replicaUP = 0;
}

class Z {
  use ZTrait;

  <<Policied("PUBLIC")>>
  public int $replicaPP = 0;

  public int $replicaPU = 0;

  <<Policied("PUBLIC")>>
  public int $replicaUP = 0;
}
