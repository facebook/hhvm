<?hh

module MLT_A;

// Public traits cannot define internal properites

trait T {
  public int $x1;
  private int $x2;
  protected int $x3;
  internal int $x4;
}
