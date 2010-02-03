package hphp;

public class HphpBoolean extends HphpVariant {
  private static HphpBoolean phpTrue = null;
  private static HphpBoolean phpFalse = null;

  public static HphpBoolean phpTrue() {
    if (phpTrue == null) {
      phpTrue = new HphpBoolean(true);
    }
    return phpTrue;
  }

  public static HphpBoolean phpFalse() {
    if (phpFalse == null) {
      phpFalse = new HphpBoolean(false);
    }
    return phpFalse;
  }

  protected boolean value;
  
  public HphpBoolean(boolean value) {
    super(Hphp.buildVariant(HPHP_BOOLEAN, value ? 1 : 0, 0));
    this.value = value;
  }
  
  protected HphpBoolean(long ptr, boolean value) {
    super(ptr);
    this.value = value;
  }
  
  public boolean getValue() {
    return this.value;
  }
  
  public String toString() {
    return Boolean.toString(this.value);
  }
}
