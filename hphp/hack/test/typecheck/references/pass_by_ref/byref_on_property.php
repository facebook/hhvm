<?hh // partial
// Copyright 2004-present Facebook. All Rights Reserved.

 class A {
   public int $prop = 10;
   public static int $prop2 = 10;

   public function methodA(int &$ref): void {
     $ref++;
   }

   public function methodB(): void {
     $x = 1;
     // ok
     $this->methodA(&$x);
     // error
     $this->methodA(&$this->prop);
     // error
     $this->methodA(&self::$prop2);
   }
 }
