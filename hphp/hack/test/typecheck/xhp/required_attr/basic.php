<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class :a extends XHPTest { attribute int a @required;}
class :b extends XHPTest { attribute int a @lateinit;}
class :c extends XHPTest { attribute int a;}
class :d extends XHPTest {
  attribute
    int a @required,
    int b @lateinit,
    int c;
}
class :e extends XHPTest { attribute :a;}
class :f extends XHPTest { attribute :b;}

function bar(): void {
  // No error
  $a = <a a={1} />;
  $a = <b />;
  $a = <c />;
  $a = <f />;

  // Error
  $a = <a />;
  $a = <d />;
  $a = <e />;
}
