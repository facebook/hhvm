package hphp;

/**
 * The class that handles HPHP system calls.
 *
 * @author qixin
 */
public class Hphp {
  public static native void includeFile(String file);

  /**
   * Invokes a top-level PHP function.
   */
  public static HphpVariant invoke(String func, HphpArray args) {
    return invokeFunction(func, args.ptr);
  }

  /**
   * Invokes a static class method.
   */
  public static HphpVariant invoke(String cls, String func, HphpArray args) {
    return invokeStaticMethod(cls, func, args.ptr);
  }

  /**
   * Creates an object.
   */
  public static HphpObject createObject(String cls, HphpArray args) {
    return (HphpObject)createObject(cls, args.ptr);
  }

////////////////////////////////////////////////////////////////////////////////

  private static boolean hphpInitialized = false;
  private static HphpSession session = null;

  protected static synchronized void startHphp() {
    if (!hphpInitialized) {
      hphpInitialized = true;
      ffiProcessInit();
    }
  }

  /**
   * Starts an hphp session, which is itself a thread.
   */
  protected static synchronized void startSession() throws RuntimeException {
    if (session != null) {
      // There can only be one session at a time.
      throw new RuntimeException("Cannot start a second Hphp session.");
    }
    ffiSessionInit();
    session = (HphpSession)Thread.currentThread();
  }

  /**
   * Finishes an hphp session.
   */
  protected static synchronized void finishSession() {
    System.gc();
    System.runFinalization();
    ffiSessionExit();
    session = null;
  }

  protected static HphpSession getSession() {
    return session;
  }

  protected static HphpNull createHphpNull(long ptr) {
    return new HphpNull(ptr);
  }
  
  protected static HphpBoolean createHphpBoolean(long ptr, boolean value) {
    return new HphpBoolean(ptr, value);
  }
  
  protected static HphpInt64 createHphpInt64(long ptr, long value) {
    return new HphpInt64(ptr, value);
  }
  
  protected static HphpDouble createHphpDouble(long ptr, double value) {
    return new HphpDouble(ptr, value);
  }
  
  protected static HphpString createHphpString(long ptr) {
    return new HphpString(ptr);
  }
  
  protected static HphpArray createHphpArray(long ptr) {
    return new HphpArray(ptr);
  }
  
  protected static HphpObject createHphpObject(long ptr) {
    return new HphpObject(ptr);
  }
  
  protected static native long buildVariant(int kind, long v, int len);
  
  protected static native void freeVariant(long ptr);
  
  protected static native long getIterBegin(long ptr);
  
  protected static native long getIterAdvanced(long ptr, long pos);
  
  protected static native boolean isIterValid(long ptr, long pos);
  
  protected static native HphpVariant getKey(long arrPtr, long pos);
  
  protected static native HphpVariant getValue(long arrPtr, long pos);
  
  protected static native HphpVariant invokeFunction(String func, 
                                                     long argsPtr);
  
  protected static native HphpVariant invokeStaticMethod(String cls, 
                                                         String func,
                                                         long argsPtr);
  
  protected static native HphpVariant invokeMethod(long targetPtr, String func,
                                                   long argsPtr);

  protected static native HphpVariant createObject(String cls, long argsPtr);
  
  protected static native void set(long map, long key, long value);

  protected static native HphpVariant get(long map, long key);

  protected static native long createHphpString(String str);

  protected static native void ffiProcessInit();
  protected static native void ffiSessionInit();
  protected static native void ffiSessionExit();
}
