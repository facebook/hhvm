<?php

/*
   +-------------------------------------------------------------+
   | Copyright (c) 2014 Facebook, Inc. (http://www.facebook.com) |
   +-------------------------------------------------------------+
*/

error_reporting(-1);

interface XIterator extends Traversable
{
    function current();
    function key();
    function next();
    function rewind();
    function valid();
}
