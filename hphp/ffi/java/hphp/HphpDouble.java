package hphp;

public class HphpDouble extends HphpVariant {
  protected double value;
  
  public HphpDouble(double value) {
    super(Hphp.buildVariant(HPHP_DOUBLE, 
                            Double.doubleToRawLongBits(value), 0));
    this.value = value; 
  }
  
  protected HphpDouble(long ptr, double value) {
    super(ptr);
    this.value = value;
  }
  
  public double getValue() {
    return value;
  }
  
  public String toString() {
    return Double.toString(this.value);
  }
}
