package hphp;

public class HphpArray extends HphpVariant {
  protected HphpArray(long ptr) {
    super(ptr);
  }
  
  public HphpArray() {
    super(Hphp.buildVariant(HPHP_EMPTY_ARRAY, 0, 0));
  }

  public HphpArray(HphpVariant[] arr) {
    this();
    for (HphpVariant v : arr) {
      append(v);
    }
  }

  public HphpArray(HphpVariant v0) {
    this();
    append(v0);
  }

  public HphpArray(HphpVariant v0, HphpVariant v1) {
    this(v0);
    append(v1);
  }

  public HphpArray(HphpVariant v0, HphpVariant v1, HphpVariant v2) {
    this(v0, v1);
    append(v2);
  }

  public HphpArray(HphpVariant v0, HphpVariant v1, HphpVariant v2, 
                   HphpVariant v3) {
    this(v0, v1, v2);
    append(v3);
  }

  public HphpArray(HphpVariant v0, HphpVariant v1, HphpVariant v2,
                   HphpVariant v3, HphpVariant v4) {
    this(v0, v1, v2, v3);
    append(v4);
  }

  public HphpArray(HphpVariant v0, HphpVariant v1, HphpVariant v2,
                   HphpVariant v3, HphpVariant v4, HphpVariant v5) {
    this(v0, v1, v2, v3, v4);
    append(v5);
  }

  public void set(HphpVariant key, HphpVariant value) {
    Hphp.set(this.ptr, key.ptr, value.ptr);
  }

  public HphpVariant get(HphpVariant key) {
    return Hphp.get(this.ptr, key.ptr);
  }

  /**
   * $arr[] = $v
   */
  public HphpArray append(HphpVariant value) {
    append(ptr, value.ptr);
    return this;
  }

  protected native void append(long arrPtr, long valPtr);

  public HphpArrayIterator iterator() {
    return new HphpArrayIterator(this);
  }
}
