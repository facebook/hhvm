<?php

/*
   +-------------------------------------------------------------+
   | Copyright (c) 2014 Facebook, Inc. (http://www.facebook.com) |
   +-------------------------------------------------------------+
*/

error_reporting(-1);

function incdec($a)
{
    echo "--------------------------------------- start incdec ---\n";
    echo '$a = '.$a." <---> "; var_dump($a);

    $a--;
    echo '$a = '.$a." <---> "; var_dump($a);

    $a++;
    echo '$a = '.$a." <---> "; var_dump($a);

    echo '$a = '.$a++."\n";
    echo '$a = '.$a." <---> "; var_dump($a);
    echo "--------------------------------------- end incdec ---\n";
}

function incdecrev($a)
{
    echo "--------------------------------------- start incdecrev ---\n";
    echo '$a = '.$a." <---> "; var_dump($a);

    $a++;
    echo '$a = '.$a." <---> "; var_dump($a);

    $a--;
    echo '$a = '.$a." <---> "; var_dump($a);

    echo '$a = '.$a--."\n";
    echo '$a = '.$a." <---> "; var_dump($a);
    echo "--------------------------------------- end incdecrev ---\n";
}
///*
// integer values ----------------------------------------------------

incdec(5);
incdecrev(5);
incdec(-10);
incdecrev(-10);
incdec(PHP_INT_MAX);
incdecrev(PHP_INT_MAX);
incdec(-PHP_INT_MAX - 1);
incdecrev(-PHP_INT_MAX - 1);

// floating-point values ----------------------------------------------------

incdec(12.345);
incdecrev(12.345);
//*/

///*
// special IEEE floating-point values ----------------------------------------------------

incdec(INF);
incdecrev(INF);
incdec(-INF);
incdecrev(-INF);
incdec(NAN);
incdecrev(NAN);
//*/

///*
// NULL value ----------------------------------------------------

incdec(NULL);
incdecrev(NULL);
//*/

///*
// Boolean values ----------------------------------------------------

incdec(TRUE);
incdecrev(FALSE);
//*/

// string values ----------------------------------------------------
///*
// an empty string

incdec("");
incdecrev("");
//*/

// strings containing numbers
///*
// whole-numbers

incdec("0");
incdecrev("0");
incdec("9");
incdecrev("9");
incdec("26");
incdecrev("26");
incdec("98325");
incdecrev("98325");
incdec("9223372036854775807");
incdecrev("9223372036854775807");
//*/

///*
// test if number bases other than decimal are supported

incdec("012");
incdecrev("012");
incdec("0x12");
incdecrev("0x12");
incdec("0X12");
incdecrev("0X12");
incdec("0b101");
incdecrev("0b101");
incdec("0B101");
incdecrev("0B101");
incdec("0Q101");
incdecrev("0Q101");
//*/

///*
// fractional values

incdec("123.456");
incdecrev("123.456");
incdec("1.23E-27");
incdecrev("1.23E-27");
//*/

///*
// leading and trailing whitespace

incdec(" 43");
incdecrev(" 43");
incdec("   654");
incdecrev("   654");
incdec("\t \n\f\r\v94");
incdecrev("\t \n\f\r\v94");
incdec("987 ");
incdecrev("987 ");
incdec("15 \t \n\f\r\v");
incdecrev("15 \t \n\f\r\v");
//*/

///*
// strings with leading zeros

incdec("012");
incdecrev("012");
incdec("   000012345");
incdecrev("   000012345");
incdec("00012.345");
incdecrev("00012.345");
incdec("  00012.345");
incdecrev("  00012.345");
//*/

///*
// leading sign

incdec("-12345");
incdecrev("-12345");
incdec("+9.87");
incdecrev("+9.87");
//*/

// string containing non-numbers
///*
// strings containing one alphabetic character

incdec("a");
incdecrev("a");
incdec("z");
incdecrev("z");

incdec("A");
incdecrev("A");
incdec("Z");
incdecrev("Z");
//*/

///*
// strings containing multiple alphanumeric characters

incdec("F28");
incdecrev("F28");
incdec("F28");
incdecrev("F98");
incdec("F98");
incdecrev("FZ8");
incdec("ZZ8");
incdecrev("ZZ8");
incdecrev("543J");
incdec("543J");
incdecrev("543J9");
incdec("543J9");
//*/

///*
// strings ending in non-alphanumeric characters

incdec("&");
incdecrev("&");
incdec("83&");
incdecrev("83&");
incdec("83&8");
incdecrev("83&8");
incdec("83&Z8");
incdecrev("83&Z8");
incdec("83&z8");
incdecrev("83&z8");
incdec("&28");
incdecrev("&28");
incdec("&98");
incdecrev("&98");
//*/

///*
$x = "aa";
var_dump($x);
var_dump(--$x);
var_dump(--$x);

$x = "zza";
var_dump($x);
var_dump(--$x);
var_dump(--$x);
//*/
