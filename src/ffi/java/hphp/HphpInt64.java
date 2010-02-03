package hphp;

public class HphpInt64 extends HphpVariant {
  protected long value;
  
  public HphpInt64(long value) {
    super(Hphp.buildVariant(HPHP_INT64, value, 0));
    this.value = value;
  }
  
  protected HphpInt64(long ptr, long value) {
    super(ptr);
    this.value = value;
  }
  
  public long getValue() {
    return this.value;
  }
  
  public String toString() {
    return Long.toString(this.value);
  }
}
