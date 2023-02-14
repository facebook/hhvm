<?hh

trait TA {
  public    int $t_public      = 1;
  protected int $t_protected   = 1;
  private   int $t_private     = 1;

  public    int $ta_public     = 1;
  protected int $ta_protected  = 1;
  private   int $ta_private    = 1;
}

trait TB {
  public    int $t_public      = 1;
  protected int $t_protected   = 1;
  private   int $t_private     = 1;

  public    int $tb_public     = 1;
  protected int $tb_protected  = 1;
  private   int $tb_private    = 1;
}

trait TC {
  public    int $t_public      = 1;
  protected int $t_protected   = 1;
  private   int $t_private     = 1;

  public    int $tc_public     = 1;
  protected int $tc_protected  = 1;
  private   int $tc_private    = 1;
}

trait TD {
  use TC;

  public    int $t_public      = 1;
  protected int $t_protected   = 1;
  private   int $t_private     = 1;

  public    int $td_public     = 1;
  protected int $td_protected  = 1;
  private   int $td_private    = 1;
}

class A {
  use TA;

  public    int $a_public      = 1;
  protected int $a_protected   = 1;
  private   int $a_private     = 1;

  public    int $ab_public     = 2;
  protected int $ab_protected  = 2;
  private   int $ab_private    = 2;

  public    int $ac_public     = 3;
  protected int $ac_protected  = 3;
  private   int $ac_private    = 3;

  public    int $abc_public    = 4;
  protected int $abc_protected = 4;
  private   int $abc_private   = 4;

  public    int $ta_public     = 1;
  protected int $ta_protected  = 1;
  private   int $ta_private    = 1;

  private   int $shared        = 1;
}

class B extends A {
  use TB;

  public    int $b_public      = 5;
  protected int $b_protected   = 5;
  private   int $b_private     = 5;

  public    int $bc_public     = 6;
  protected int $bc_protected  = 6;
  private   int $bc_private    = 6;

  public    int $ab_public     = 7;
  protected int $ab_protected  = 7;
  private   int $ab_private    = 7;

  public    int $abc_public    = 8;
  protected int $abc_protected = 8;
  private   int $abc_private   = 8;

  protected int $shared        = 1;
}

class C extends B {
  use TC;

  public    int $c_public      = 9;
  protected int $c_protected   = 9;
  private   int $c_private     = 9;

  public    int $bc_public     = 10;
  protected int $bc_protected  = 10;
  private   int $bc_private    = 10;

  public    int $ac_public     = 11;
  protected int $ac_protected  = 11;
  private   int $ac_private    = 11;

  public    int $abc_public    = 12;
  protected int $abc_protected = 12;
  private   int $abc_private   = 12;

  public int $shared           = 1;
}

class D extends C {
  use TD;
}

<<__EntryPoint>>
function main(): void {
  echo serialize(new A())."\n";
  echo serialize(new B())."\n";
  echo serialize(new C())."\n";
  echo serialize(new D())."\n";
}
