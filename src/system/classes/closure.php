<?php

/**
 * Internal class. Should NOT use directly at all.
 */
class Closure {
  protected $func = '';
  protected $vars = array();

  public function __construct($func, $vars) {
    $this->func = $func;
    $this->vars = $vars;
  }

  public function __toString() {
    return $this->func . ':' . hphp_object_pointer($this);
  }

  public function getVars() {
    return $this->vars;
  }
  public function setVars($vars) {
    unset($vars["__cont__"]);
    $this->vars = $vars;
  }
}

/**
 * Internal class. Should NOT use directly at all.
 */
class Continuation extends Closure implements Iterator {
  private $obj;
  public $label = 0;
  private $done = false;
  private $index = 0;
  private $value;

  public function __construct($func, $vars, $obj = null) {
    parent::__construct($func, $vars);
    $this->obj = $obj;
  }
  public function done() {
    $this->done = true;
  }

  public function current() {
    return $this->value;
  }
  public function key() {
    return $this->index;
  }
  public function next() {
    ++$this->index;
    if ($this->obj) {
      $func = $this->func;
      $this->value = $this->obj->$func($this);
    } else {
      $this->value = call_user_func($this->func, $this);
    }
  }
  public function rewind() {
    $this->label = '';
    $this->index = -1;
    $this->next();
  }
  public function valid() {
    return !$this->done;
  }
}
