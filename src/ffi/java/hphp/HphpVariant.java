package hphp;

public abstract class HphpVariant {
  final protected static int HPHP_NULL = 0;
  final protected static int HPHP_BOOLEAN = 1;
  final protected static int HPHP_INT64 = 2;
  final protected static int HPHP_DOUBLE = 3;
  final protected static int HPHP_STRING = 5;
  final protected static int HPHP_ARRAY = 7;
  final protected static int HPHP_OBJECT = 8;
  final protected static int HPHP_EMPTY_ARRAY = 9;
  
  protected long ptr; // pointer value to the underlying hphp variant
  protected HphpSession session;

  protected HphpVariant(long ptr) {
    this.session = Hphp.getSession();
    this.ptr = ptr;
  }

  public long getVariantPtr() {
    return this.ptr;
  }
}
