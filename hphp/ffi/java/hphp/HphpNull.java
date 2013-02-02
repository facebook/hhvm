package hphp;

public class HphpNull extends HphpVariant {
  private static HphpNull phpNull = null;

  public static HphpNull phpNull() {
    if (phpNull == null) {
      phpNull = new HphpNull();
    }
    return phpNull;
  }

  public HphpNull() {
    super(Hphp.buildVariant(HPHP_NULL, 0, 0));
  }
  
  protected HphpNull(long ptr) {
    super(ptr);
  }
  
  public String toString() {
    return "null";
  }
}
