package hphp;

/**
 * Wrapper of an HPHP string.
 */
public class HphpString extends HphpVariant {
  protected String str;

  public HphpString(String str) {
    super(Hphp.createHphpString(str));
    this.str = str;
  }

  protected HphpString(long ptr) {
    super(ptr);
    this.str = getHphpString(ptr);
  }

//  protected static native long createHphpString(String str);

  protected native String getHphpString(long ptr);

  public String toString() {
    return str;
  }
}
