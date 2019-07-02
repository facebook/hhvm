<?hh // strict

abstract class A {
  public abstract const int MAX_REGS, MIN_REGS; // ok
  private abstract const bool BLOCK_REORDERING, TRACE; // illegal
}
