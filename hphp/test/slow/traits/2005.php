<?php

class C1 {
}
trait T1 {
}
interface I1 {
}
var_dump(class_implements('C1'));
var_dump(class_parents('C1'));
var_dump(class_uses('C1'));
var_dump(class_implements('I1'));
var_dump(class_parents('I1'));
var_dump(class_uses('I1'));
var_dump(class_implements('T1'));
var_dump(class_parents('T1'));
var_dump(class_uses('T1'));
