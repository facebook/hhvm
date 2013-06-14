<?php

$ret = null;
var_dump(fb_thrift_unserialize(fb_thrift_serialize("test"), $ret));
var_dump($ret);

$ret = null;
var_dump(fb_thrift_unserialize(fb_thrift_serialize(array("test")), $ret));
var_dump($ret);
