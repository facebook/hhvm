<?php

class Directory {
  public $path;
  public $handle;

  public function __construct($path) {
    $this->path = $path;
    $this->handle = opendir($path);
  }

  public function read() {
    return readdir($this->handle);
  }

  public function rewind() {
    return rewinddir($this->handle);
  }

  public function close() {
    return closedir($this->handle);
  }
}
