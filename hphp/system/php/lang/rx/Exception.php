<?php

namespace HH\Rx;

interface Exception {
  require extends \Exception;
  <<__Rx>>
  public function getMessage(): string;
  <<__Rx>>
  public function getCode(): int;
}
