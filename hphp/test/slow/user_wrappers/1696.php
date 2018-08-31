<?php

class PrivateWrapper {
  private function stream_open($fn, $mode, $opt, &$opened_path) {
    return true;
  }
  public static function openme() {
    return fopen('pw://foo', 'r');
  }
}

<<__EntryPoint>>
function main_1696() {
stream_wrapper_register('pw', 'PrivateWrapper');
var_dump(is_resource(PrivateWrapper::openme()));
}
