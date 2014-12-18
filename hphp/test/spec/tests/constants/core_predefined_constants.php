<?php

/*
   +-------------------------------------------------------------+
   | Copyright (c) 2014 Facebook, Inc. (http://www.facebook.com) |
   +-------------------------------------------------------------+
*/

error_reporting(-1);

function trace($text, $pdc)
{
    echo "$text: ";
    var_dump($pdc);
}

trace("__LINE__", __LINE__);

trace("__FILE__", __FILE__);

trace("__DIR__", __DIR__);
var_dump(dirname(__FILE__));

trace("__LINE__", __LINE__);

trace("__NAMESPACE__", __NAMESPACE__);

echo "-----------------------------------------\n";

echo "At the top level of a script\n";
trace("__FUNCTION__", __FUNCTION__);

echo "-----------------------------------------\n";

echo "At the top level of a script and outside all classes\n";
trace("__METHOD__", __METHOD__);

echo "-----------------------------------------\n";

echo "Outside all classes\n";
trace("__CLASS__", __CLASS__);

echo "-----------------------------------------\n";

echo "Outside all classes\n";
trace("__TRAIT__", __TRAIT__);

echo "-----------------------------------------\n";

function ComputeResult()
{
    echo "Inside ComputeResult\n";
    trace("__FUNCTION__", __FUNCTION__);
    trace("__METHOD__", __METHOD__);
    trace("__CLASS__", __CLASS__);
    trace("__TRAIT__", __TRAIT__);
    trace("__NAMESPACE__", __NAMESPACE__);

    function Inner()
    {
        echo "Inside ComputeResult\n";
        trace("__FUNCTION__", __FUNCTION__);
        trace("__METHOD__", __METHOD__);
    }

    Inner();
}

ComputeResult();

echo "-----------------------------------------\n";

class Date
{
    public function __construct()
    {
        echo "Inside " . __METHOD__ . "\n";
        trace("__CLASS__", __CLASS__);
        trace("__FUNCTION__", __FUNCTION__);
        trace("__TRAIT__", __TRAIT__);
        trace("__NAMESPACE__", __NAMESPACE__);

        // ...
    }

    function __destruct()
    {
        echo "Inside " . __METHOD__ . "\n";
        trace("__FUNCTION__", __FUNCTION__);

        // ...
    }

    public function setDay($day)
    {
        echo "Inside " . __METHOD__ . "\n";
        trace("__FUNCTION__", __FUNCTION__);
        trace("__LINE__", __LINE__);

        $this->priv1();
        $this->spf1();
    }

// public vs. private doesn't matter

    private function priv1()
    {
        echo "Inside " . __METHOD__ . "\n";
        trace("__FUNCTION__", __FUNCTION__);
    }

    static public function spf1()
    {
        echo "Inside " . __METHOD__ . "\n";
        trace("__FUNCTION__", __FUNCTION__);
    }

}

$date1 = new Date;
$date1->setDay(22);

echo "-----------------------------------------\n";

class DatePlus extends Date
{
    public function xx()
    {
        trace("__CLASS__", __CLASS__);
        echo "Inside " . __METHOD__ . "\n";
        trace("__FUNCTION__", __FUNCTION__);
    }
}

$datePlus1 = new DatePlus;
$datePlus1->xx();

include_once('includefile.inc');
