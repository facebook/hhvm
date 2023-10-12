<?hh // strict

trait R {
  use T;

  <<__Policied("PUBLIC")>>
  public int $rPub = 0;

  <<__Policied("PUBLIC")>>
  private int $rPri = 0;

  <<__Policied("PUBLIC")>>
  protected int $rPro = 0;

  public int $rUnpolicied = 0;
}

class C extends B {
  use T;

  <<__Policied("PUBLIC")>>
  public int $cPub = 0;

  <<__Policied("PUBLIC")>>
  private int $cPri = 0;

  <<__Policied("PUBLIC")>>
  protected int $cPro = 0;

  public int $cUnpolicied = 0;
}

class A {
  <<__Policied("PUBLIC")>>
  public int $aPub = 0;

  <<__Policied("PUBLIC")>>
  private int $aPri = 0;

  <<__Policied("PUBLIC")>>
  protected int $aPro = 0;

  public int $aUnpolicied = 0;
}

class B extends A {
  <<__Policied("PUBLIC")>>
  public int $bPub = 0;

  <<__Policied("PUBLIC")>>
  private int $bPri = 0;

  <<__Policied("PUBLIC")>>
  protected int $bPro = 0;

  public int $bUnpolicied = 0;
}

trait T {
  <<__Policied("PUBLIC")>>
  public int $tPub = 0;

  <<__Policied("PUBLIC")>>
  private int $tPri = 0;

  <<__Policied("PUBLIC")>>
  protected int $tPro = 0;

  public int $tUnpolicied = 0;
}

trait ZTrait {
  <<__Policied("PUBLIC")>>
  public int $replicaPP = 0;

  <<__Policied("PUBLIC")>>
  public int $replicaPU = 0;

  public int $replicaUP = 0;
}

class Z {
  use ZTrait;

  <<__Policied("PUBLIC")>>
  public int $replicaPP = 0;

  public int $replicaPU = 0;

  <<__Policied("PUBLIC")>>
  public int $replicaUP = 0;
}
