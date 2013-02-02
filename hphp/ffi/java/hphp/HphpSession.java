package hphp;

public class HphpSession extends Thread {
  private Runnable work;

  public HphpSession(Runnable work) {
    this.work = work;
    // In case it is not called already.
    Hphp.startHphp();
  }

  public void run() {
    Hphp.startSession();
    work.run();
    Hphp.finishSession();
  }

  public void start() {
    super.start();
    try {
      // Let the main thread wait for the current session to finish.
      this.join();
    } catch (InterruptedException e) {
    }
  }
}
