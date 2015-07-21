<?php

/*
   +-------------------------------------------------------------+
   | Copyright (c) 2015 Facebook, Inc. (http://www.facebook.com) |
   +-------------------------------------------------------------+
*/

error_reporting(-1);

class XGenerator implements Iterator
{
    public function current() {}
    public function key() {}
    public function next() {}
    public function rewind() {}
    public function send($value) {}
//  public function throw(Exception $exception) {}
    public function valid() {}
    public function __wakeup() {}
}
