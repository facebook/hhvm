<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

class :a { attribute int a @required;}
class :b { attribute int a @lateinit;}
class :c { attribute int a;}
class :d {
  attribute
    int a @required,
    int b @lateinit,
    int c;
}
class :e { attribute :a;}
class :f { attribute :b;}

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
