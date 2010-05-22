<?php

class Exception {
  protected $message = 'Unknown exception';   // exception message
  protected $code = 0;                        // user defined exception code
  protected $file;                            // source filename of exception
  protected $line;                            // source line of exception

  function __construct($message = '', $code = 0) {
    $this->message = $message;
    $this->code = $code;
    $this->trace = debug_backtrace();

    // removing exception constructor stacks to be consistent with PHP
    while (!empty($this->trace)) {
      $top = $this->trace[0];
      if (empty($top['class']) ||
          (strcasecmp($top['function'], '__construct') &&
           strcasecmp($top['function'], $top['class'])) ||
          (strcasecmp($top['class'], 'exception') &&
           !is_subclass_of($top['class'], 'exception'))) {
        break;
      }
      $frame = array_shift($this->trace);
    }

    $this->file = $frame['file'];
    $this->line = $frame['line'];
  }

  // message of exception
  final function getMessage() {
    return $this->message;
  }

  // code of exception
  final function getCode() {
    return $this->code;
  }

  // source filename
  final function getFile() {
    return $this->file;
  }

  // source line
  final function getLine() {
    return $this->line;
  }

  // an array of the backtrace()
  final function getTrace() {
    return $this->trace;
  }

  // formated string of trace
  final function getTraceAsString() {
    // works with the new FrameInjection-based stacktrace.
    $i = 0;
    $s = "";
    foreach ($this->getTrace() as $frame) {
      if (!is_array($frame)) continue;
      $s .= "#$i " . $frame['file'] . "(" .
            $frame['line']. "): " .
            (isset($frame['class']) ? $frame['class'] . $frame['type'] : "") .
            $frame['function'] . "()\n";
      $i++;
    }
    $s .= "#$i {main}";
    return $s;
  }

  /* Overrideable */
  // formated string for display
  function __toString() {
    return $this->getMessage();
  }
}

class LogicException extends Exception {}
  class BadFunctionCallException extends LogicException {}
    class BadMethodCallException extends BadFunctionCallException {}
  class DomainException          extends LogicException {}
  class InvalidArgumentException extends LogicException {}
  class LengthException          extends LogicException {}
  class OutOfRangeException      extends LogicException {}
class RuntimeException extends Exception {}
  class OutOfBoundsException     extends RuntimeException {}
  class OverflowException        extends RuntimeException {}
  class RangeException           extends RuntimeException {}
  class UnderflowException       extends RuntimeException {}
  class UnexpectedValueException extends RuntimeException {}

class ErrorException extends Exception {
  protected $severity;
  public function __construct($message = "", $code = 0, $severity = 0,
                              $filename = null, $lineno = null) {
    parent::__construct($message, $code);
    $this->severity = $severity;
    if ($filename !== null) {
      $this->file = $filename;
    }
    if ($lineno !== null) {
      $this->line = $lineno;
    }
  }

  final public function getSeverity() { return $this->severity; }
}
