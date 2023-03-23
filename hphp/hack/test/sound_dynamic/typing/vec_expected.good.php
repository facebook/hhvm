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
