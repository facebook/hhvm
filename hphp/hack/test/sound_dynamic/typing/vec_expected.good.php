<?hh
// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

enum E : int {
  A = 1;
  B = 2;
}

class CL {
const vec<vec<E>>
    CONST = vec[
      vec[
        E::A,
        E::B,
      ],
    ];
}

class CL2 {
const vec<vec<int>>
    CONST = vec[
      vec[
        2,
        3,
      ],
    ];
}


function expectVecVec(vec<vec<int>> $_):void { }

function test1():void {
  expectVecVec(vec<vec<int>>[vec[2,3]]);

  // Generates vec<int>[2:int,3:int]:vec<int>
  $a = vec<int>[2,3];
  hh_expect_equivalent<vec<int>>($a);
  // Generates vec[2:int,3:int]:vec<int>
  $b = vec[2,3];
  hh_expect_equivalent<vec<int>>($b);
  // Generates vec<vec<int>>[vec[2:int,3:int]:vec<~int>]:vec<vec<~int>>
  $c = vec<vec<int>>[vec[2,3]];
  hh_expect_equivalent<vec<vec<int>>>($c);
  // Generates vec[vec[2:int,3:int]:vec<int>]:vec<vec<int>>
  $d = vec[vec[2,3]];
  hh_expect_equivalent<vec<vec<int>>>($d);
}
