<?php
interface I { function foo(); }
trait T0 implements I {}
trait T implements I { use T0; }
abstract class X implements I { use T; }
abstract class Y implements I { use T; }
