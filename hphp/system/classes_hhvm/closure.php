<?php

/**
 * Used as the base class for all closures
 */
class Closure {
  // The bound $this for the closure. Could be null.
  protected $this;
  // The context class for calling $functionName when $this is null
  protected $className;
  // The function to call on $this (or $className)
  protected $functionName;

  // For storing the static locals from the closure body
  protected $__static_locals;

  // All the variables from the use statement will be private variables on the
  // subclasses so they don't have to be packaged in an array and then back out
  // on every call

  /**
   * Adding a dummy __sleep() to return an illegal value to make the code
   * go through error handling path
   */
  public function __sleep() {
    return false;
  }

  /**
   * This is handled by each subclass basically inlining getUseVars()
   * and skipping all the overhead of call_user_func_array
  public function __invoke() {
    $context = $this->this ?: $this->className;
    call_user_func_array(
      array($context, $this->functionName),
      func_get_args() + $this->getUseVars()
    );
  }
  */
}
