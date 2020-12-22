<?hh

class Foo {
  const vec<vec<int>> constNested = vec[
    HH\array_mark_legacy(vec[]),
  ];

  public static vec<vec<int>> $spropNested = vec[
    HH\array_mark_legacy(vec[]),
  ];

  public vec<vec<int>> $propNested = vec[
    HH\array_mark_legacy(vec[]),
  ];

}

const vec<vec<int>> gconstNested = vec[
  HH\array_mark_legacy(vec[]),
];

const gconstWithoutHintNested = vec[
  HH\array_mark_legacy(vec[]),
];

record recordNested {
  vec<vec<int>> a = vec[
    HH\array_mark_legacy(vec[]),
  ];
}
