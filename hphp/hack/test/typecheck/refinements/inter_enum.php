<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

enum E: int {
  A = 0;
  B = 1;
}

enum F: string {
  C = 'C';
  D = 'D';
}

enum G: int {
  A = 0;
  B = 1;
  C = 2;
}

function test1(E $e): void {
  if ($e is F) {
    switch ($e) {
      case E::A:
        // code...
        break;
      case E::B:
        // ...
        break;
    }
  }
}

function test2(G $e): void {
  if ($e is E) {
    switch ($e) {
      case E::A:
        // code...
        break;
      case E::B:
        // ...
        break;
    }
  }
}
