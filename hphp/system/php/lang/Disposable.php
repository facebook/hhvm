<?php

interface IDisposable {
  public function __dispose();
}

interface IAsyncDisposable {
  public function __disposeAsync();
}
