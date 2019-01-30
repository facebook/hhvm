<?php

/*
   +-------------------------------------------------------------+
   | Copyright (c) 2015 Facebook, Inc. (http://www.facebook.com) |
   +-------------------------------------------------------------+
*/

error_reporting(-1);

interface XArrayAccess
{
    function offsetExists ($offset);
    function offsetGet ($offset);
    function offsetSet ($offset, $value);
    function offsetUnset ($offset);
}
