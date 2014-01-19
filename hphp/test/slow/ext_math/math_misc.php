<?php

function VC($x, $y) {
  var_dump(abs($x - $y) < 0.00001);
}

var_dump(pi());
var_dump(M_PI);

var_dump(is_finite(5));
var_dump(is_finite(log(0)));
var_dump(is_nan(acos(1.01)));

VC(deg2rad(45), M_PI_4);
VC(rad2deg(M_PI_4), 45);

var_dump(decbin(12));
var_dump(decbin(26));
var_dump(dechex(10));
var_dump(dechex(47));

var_dump(decoct(15));
var_dump(decoct(264));

var_dump(bindec("110011"));
var_dump(bindec("000110011"));
var_dump(bindec("111"));

var_dump(hexdec("See"));
var_dump(hexdec("ee"));
var_dump(hexdec("that"));
var_dump(hexdec("a0"));

var_dump(octdec("77"));
var_dump(octdec(decoct(45)));

var_dump(base_convert("A37334", 16, 2));


VC(pow(2, 8), 256);
VC(pow(-1, 20), 1);
var_dump(pow(0, 0));
var_dump(is_int(pow(2, 32)));
VC(pow("2", "8"), 256);
VC(pow("-1", "20"), 1);
var_dump(pow("0", "0"));
var_dump(is_int(pow("2", "32")));

VC(exp(12), 162754.791419);
VC(exp(5.7), 298.86740096706);

VC(expm1(5.7), 297.8674);

var_dump(log10(10));
var_dump(log10(100));

VC(log1p(9), 2.302585092994);
VC(log1p(99), 4.6051701859881);

VC(log(8), 2.0794415416798);
var_dump(log(8, 2));

VC(cos(M_PI), -1);
VC(cosh(1.23), 1.8567610569853);
VC(sin(deg2rad(90)), 1);
VC(sinh(1.23), 1.5644684793044);
VC(tan(deg2rad(45)), 1);
VC(tanh(1.23), 0.84257932565893);
VC(acos(-1), M_PI);
VC(acosh(1.8567610569853), 1.23);
VC(asin(1), deg2rad(90));
VC(asinh(1.5644684793044), 1.23);
VC(atan(1), deg2rad(45));
VC(atanh(0.84257932565893), 1.23);
VC(atan2(3, 4), 0.64350110879328);
var_dump(hypot(3, 4));
var_dump(fmod(5.7, 1.3));
var_dump(sqrt(9));
var_dump(getrandmax());

var_dump(mt_getrandmax());

