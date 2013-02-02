package hphp;

public class HphpObject extends HphpVariant {
  protected HphpObject(long ptr) {
    super(ptr);
  }

  public HphpVariant invokeHphp(String func, HphpArray args) {
    return Hphp.invokeMethod(this.ptr, func, args.ptr);
  }

  public HphpVariant getHphp(HphpVariant key) {
    return Hphp.get(this.ptr, key.ptr);
  }
}
