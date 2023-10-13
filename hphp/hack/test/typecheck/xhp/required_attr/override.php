<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class :a1             { attribute int a @required; }
class :b1 extends :a1 { attribute int a @required; }
class :c1 extends :a1 { attribute int a @lateinit;}
class :d1 extends :a1 { attribute int a; }

class :a2             { attribute int a @lateinit; }
class :b2 extends :a2 { attribute int a @required; }
class :c2 extends :a2 { attribute int a @lateinit; }
class :d2 extends :a2 { attribute int a; }

class :a3             { attribute int a; }
class :b3 extends :a3 { attribute int a @required; }
class :c3 extends :a3 { attribute int a @lateinit; }
class :d3 extends :a3 { attribute int a; }
