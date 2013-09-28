<?php

interface A {
}
 class B implements A {
}
 class C extends B implements A {
}
 $obj = new C();
