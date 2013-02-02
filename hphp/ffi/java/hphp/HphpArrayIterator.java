package hphp;

import java.util.Iterator;
import java.util.NoSuchElementException;

public class HphpArrayIterator implements Iterator<HphpVariant> {
  private HphpArray arr;
  private long pos;
  
  public HphpArrayIterator(HphpArray arr) {
    this.arr = arr;
    this.pos = Hphp.getIterBegin(arr.ptr);
  }
  
  @Override
  public boolean hasNext() {
    return Hphp.isIterValid(arr.ptr, pos);
  }

  @Override
  public HphpVariant next() {
    if (Hphp.isIterValid(arr.ptr, pos)) {
      HphpVariant result = Hphp.getValue(arr.ptr, pos);
      pos = Hphp.getIterAdvanced(arr.ptr, pos);
      return result;
    }
    
    throw new NoSuchElementException();
  }

  @Override
  public void remove() {
    throw new UnsupportedOperationException();
  }
  
  public HphpVariant getNextKey() {
    if (Hphp.isIterValid(arr.ptr, pos)) {
      return Hphp.getKey(arr.ptr, pos);
    }
    
    throw new NoSuchElementException();
  }
  
  public HphpVariant getNextValue() {
    if (Hphp.isIterValid(arr.ptr, pos)) {
      return Hphp.getValue(arr.ptr, pos);
    }
    
    throw new NoSuchElementException();
  }
}
