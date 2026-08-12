<?hh
<<file: __EnableUnstableFeatures('named_parameters')>>

// Regression test for a use-after-free in HHBBC.
//
// CallContext (hphp/hhbbc/context.h) is the key of the run-lifetime
// contextualReturnTypes map, but it used to hold its named-argument list as a
// non-owning pointer into the *caller's* decompressed bytecode, which only
// lives as long as that caller's php::WideFunc. Once a caller's analysis
// finished, every key already in the map had a dangling argNames pointer, and
// the next tbb::concurrent_hash_map lookup that re-hashed or compared a stored
// key read freed memory in CallContextHasher / operator==.
//
// Three things are needed to hit that, which is what the shape below is for:
//
//  1. Calls that actually reach contextualReturnTypes.insert. With
//     options.ContextSensitiveInterp off (the default) the only way in is the
//     retParam path -- the callee must return one of its parameters verbatim,
//     and the caller must pass enough arguments to cover that parameter's
//     index. hackc reorders parameters so that named ones come first
//     (lexicographically), so `pick`'s params are [a, b, c, prefix] and
//     retParam is 3. That is why every callsite below passes *all four*
//     arguments: a call that omits an optional named argument never gets far
//     enough to insert anything.
//
//  2. Many distinct keys, so the map grows and buckets get re-hashed. Every
//     callsite uses distinct literals, and the literal in the retParam slot
//     also has to be strictly more refined than the declared `string` type for
//     checkParam to pass.
//
//  3. Several separate caller functions, so earlier callers' WideFuncs are
//     already destroyed -- and their argNames freed -- while later callers are
//     still inserting.
//
// Named arguments are written in a different order at each callsite to confirm
// that ordering does not matter; hackc sorts them before emitting.
//
// Only exercises the bug in repo mode (`hphp/test/run <test> -r`), which is
// what runs HHBBC.

class C {
  public static function pick(
    string $prefix,
    named int $a,
    named bool $b = false,
    named string $c = '',
  ): string {
    return $prefix;
  }
}

function group0(): vec<string> {
  return vec[
    C::pick('p0_0', a=0, b=true, c='c0_0'),
    C::pick('p0_1', b=false, c='c0_1', a=1),
    C::pick('p0_2', c='c0_2', a=2, b=true),
    C::pick('p0_3', b=false, a=3, c='c0_3'),
    C::pick('p0_4', a=4, b=true, c='c0_4'),
    C::pick('p0_5', b=false, c='c0_5', a=5),
    C::pick('p0_6', c='c0_6', a=6, b=true),
    C::pick('p0_7', b=false, a=7, c='c0_7'),
    C::pick('p0_8', a=8, b=true, c='c0_8'),
    C::pick('p0_9', b=false, c='c0_9', a=9),
    C::pick('p0_10', c='c0_10', a=10, b=true),
    C::pick('p0_11', b=false, a=11, c='c0_11'),
    C::pick('p0_12', a=12, b=true, c='c0_12'),
    C::pick('p0_13', b=false, c='c0_13', a=13),
    C::pick('p0_14', c='c0_14', a=14, b=true),
    C::pick('p0_15', b=false, a=15, c='c0_15'),
  ];
}

function group1(): vec<string> {
  return vec[
    C::pick('p1_0', a=16, b=true, c='c1_0'),
    C::pick('p1_1', b=false, c='c1_1', a=17),
    C::pick('p1_2', c='c1_2', a=18, b=true),
    C::pick('p1_3', b=false, a=19, c='c1_3'),
    C::pick('p1_4', a=20, b=true, c='c1_4'),
    C::pick('p1_5', b=false, c='c1_5', a=21),
    C::pick('p1_6', c='c1_6', a=22, b=true),
    C::pick('p1_7', b=false, a=23, c='c1_7'),
    C::pick('p1_8', a=24, b=true, c='c1_8'),
    C::pick('p1_9', b=false, c='c1_9', a=25),
    C::pick('p1_10', c='c1_10', a=26, b=true),
    C::pick('p1_11', b=false, a=27, c='c1_11'),
    C::pick('p1_12', a=28, b=true, c='c1_12'),
    C::pick('p1_13', b=false, c='c1_13', a=29),
    C::pick('p1_14', c='c1_14', a=30, b=true),
    C::pick('p1_15', b=false, a=31, c='c1_15'),
  ];
}

function group2(): vec<string> {
  return vec[
    C::pick('p2_0', a=32, b=true, c='c2_0'),
    C::pick('p2_1', b=false, c='c2_1', a=33),
    C::pick('p2_2', c='c2_2', a=34, b=true),
    C::pick('p2_3', b=false, a=35, c='c2_3'),
    C::pick('p2_4', a=36, b=true, c='c2_4'),
    C::pick('p2_5', b=false, c='c2_5', a=37),
    C::pick('p2_6', c='c2_6', a=38, b=true),
    C::pick('p2_7', b=false, a=39, c='c2_7'),
    C::pick('p2_8', a=40, b=true, c='c2_8'),
    C::pick('p2_9', b=false, c='c2_9', a=41),
    C::pick('p2_10', c='c2_10', a=42, b=true),
    C::pick('p2_11', b=false, a=43, c='c2_11'),
    C::pick('p2_12', a=44, b=true, c='c2_12'),
    C::pick('p2_13', b=false, c='c2_13', a=45),
    C::pick('p2_14', c='c2_14', a=46, b=true),
    C::pick('p2_15', b=false, a=47, c='c2_15'),
  ];
}

function group3(): vec<string> {
  return vec[
    C::pick('p3_0', a=48, b=true, c='c3_0'),
    C::pick('p3_1', b=false, c='c3_1', a=49),
    C::pick('p3_2', c='c3_2', a=50, b=true),
    C::pick('p3_3', b=false, a=51, c='c3_3'),
    C::pick('p3_4', a=52, b=true, c='c3_4'),
    C::pick('p3_5', b=false, c='c3_5', a=53),
    C::pick('p3_6', c='c3_6', a=54, b=true),
    C::pick('p3_7', b=false, a=55, c='c3_7'),
    C::pick('p3_8', a=56, b=true, c='c3_8'),
    C::pick('p3_9', b=false, c='c3_9', a=57),
    C::pick('p3_10', c='c3_10', a=58, b=true),
    C::pick('p3_11', b=false, a=59, c='c3_11'),
    C::pick('p3_12', a=60, b=true, c='c3_12'),
    C::pick('p3_13', b=false, c='c3_13', a=61),
    C::pick('p3_14', c='c3_14', a=62, b=true),
    C::pick('p3_15', b=false, a=63, c='c3_15'),
  ];
}

function group4(): vec<string> {
  return vec[
    C::pick('p4_0', a=64, b=true, c='c4_0'),
    C::pick('p4_1', b=false, c='c4_1', a=65),
    C::pick('p4_2', c='c4_2', a=66, b=true),
    C::pick('p4_3', b=false, a=67, c='c4_3'),
    C::pick('p4_4', a=68, b=true, c='c4_4'),
    C::pick('p4_5', b=false, c='c4_5', a=69),
    C::pick('p4_6', c='c4_6', a=70, b=true),
    C::pick('p4_7', b=false, a=71, c='c4_7'),
    C::pick('p4_8', a=72, b=true, c='c4_8'),
    C::pick('p4_9', b=false, c='c4_9', a=73),
    C::pick('p4_10', c='c4_10', a=74, b=true),
    C::pick('p4_11', b=false, a=75, c='c4_11'),
    C::pick('p4_12', a=76, b=true, c='c4_12'),
    C::pick('p4_13', b=false, c='c4_13', a=77),
    C::pick('p4_14', c='c4_14', a=78, b=true),
    C::pick('p4_15', b=false, a=79, c='c4_15'),
  ];
}

function group5(): vec<string> {
  return vec[
    C::pick('p5_0', a=80, b=true, c='c5_0'),
    C::pick('p5_1', b=false, c='c5_1', a=81),
    C::pick('p5_2', c='c5_2', a=82, b=true),
    C::pick('p5_3', b=false, a=83, c='c5_3'),
    C::pick('p5_4', a=84, b=true, c='c5_4'),
    C::pick('p5_5', b=false, c='c5_5', a=85),
    C::pick('p5_6', c='c5_6', a=86, b=true),
    C::pick('p5_7', b=false, a=87, c='c5_7'),
    C::pick('p5_8', a=88, b=true, c='c5_8'),
    C::pick('p5_9', b=false, c='c5_9', a=89),
    C::pick('p5_10', c='c5_10', a=90, b=true),
    C::pick('p5_11', b=false, a=91, c='c5_11'),
    C::pick('p5_12', a=92, b=true, c='c5_12'),
    C::pick('p5_13', b=false, c='c5_13', a=93),
    C::pick('p5_14', c='c5_14', a=94, b=true),
    C::pick('p5_15', b=false, a=95, c='c5_15'),
  ];
}

function group6(): vec<string> {
  return vec[
    C::pick('p6_0', a=96, b=true, c='c6_0'),
    C::pick('p6_1', b=false, c='c6_1', a=97),
    C::pick('p6_2', c='c6_2', a=98, b=true),
    C::pick('p6_3', b=false, a=99, c='c6_3'),
    C::pick('p6_4', a=100, b=true, c='c6_4'),
    C::pick('p6_5', b=false, c='c6_5', a=101),
    C::pick('p6_6', c='c6_6', a=102, b=true),
    C::pick('p6_7', b=false, a=103, c='c6_7'),
    C::pick('p6_8', a=104, b=true, c='c6_8'),
    C::pick('p6_9', b=false, c='c6_9', a=105),
    C::pick('p6_10', c='c6_10', a=106, b=true),
    C::pick('p6_11', b=false, a=107, c='c6_11'),
    C::pick('p6_12', a=108, b=true, c='c6_12'),
    C::pick('p6_13', b=false, c='c6_13', a=109),
    C::pick('p6_14', c='c6_14', a=110, b=true),
    C::pick('p6_15', b=false, a=111, c='c6_15'),
  ];
}

function group7(): vec<string> {
  return vec[
    C::pick('p7_0', a=112, b=true, c='c7_0'),
    C::pick('p7_1', b=false, c='c7_1', a=113),
    C::pick('p7_2', c='c7_2', a=114, b=true),
    C::pick('p7_3', b=false, a=115, c='c7_3'),
    C::pick('p7_4', a=116, b=true, c='c7_4'),
    C::pick('p7_5', b=false, c='c7_5', a=117),
    C::pick('p7_6', c='c7_6', a=118, b=true),
    C::pick('p7_7', b=false, a=119, c='c7_7'),
    C::pick('p7_8', a=120, b=true, c='c7_8'),
    C::pick('p7_9', b=false, c='c7_9', a=121),
    C::pick('p7_10', c='c7_10', a=122, b=true),
    C::pick('p7_11', b=false, a=123, c='c7_11'),
    C::pick('p7_12', a=124, b=true, c='c7_12'),
    C::pick('p7_13', b=false, c='c7_13', a=125),
    C::pick('p7_14', c='c7_14', a=126, b=true),
    C::pick('p7_15', b=false, a=127, c='c7_15'),
  ];
}

function group8(): vec<string> {
  return vec[
    C::pick('p8_0', a=128, b=true, c='c8_0'),
    C::pick('p8_1', b=false, c='c8_1', a=129),
    C::pick('p8_2', c='c8_2', a=130, b=true),
    C::pick('p8_3', b=false, a=131, c='c8_3'),
    C::pick('p8_4', a=132, b=true, c='c8_4'),
    C::pick('p8_5', b=false, c='c8_5', a=133),
    C::pick('p8_6', c='c8_6', a=134, b=true),
    C::pick('p8_7', b=false, a=135, c='c8_7'),
    C::pick('p8_8', a=136, b=true, c='c8_8'),
    C::pick('p8_9', b=false, c='c8_9', a=137),
    C::pick('p8_10', c='c8_10', a=138, b=true),
    C::pick('p8_11', b=false, a=139, c='c8_11'),
    C::pick('p8_12', a=140, b=true, c='c8_12'),
    C::pick('p8_13', b=false, c='c8_13', a=141),
    C::pick('p8_14', c='c8_14', a=142, b=true),
    C::pick('p8_15', b=false, a=143, c='c8_15'),
  ];
}

function group9(): vec<string> {
  return vec[
    C::pick('p9_0', a=144, b=true, c='c9_0'),
    C::pick('p9_1', b=false, c='c9_1', a=145),
    C::pick('p9_2', c='c9_2', a=146, b=true),
    C::pick('p9_3', b=false, a=147, c='c9_3'),
    C::pick('p9_4', a=148, b=true, c='c9_4'),
    C::pick('p9_5', b=false, c='c9_5', a=149),
    C::pick('p9_6', c='c9_6', a=150, b=true),
    C::pick('p9_7', b=false, a=151, c='c9_7'),
    C::pick('p9_8', a=152, b=true, c='c9_8'),
    C::pick('p9_9', b=false, c='c9_9', a=153),
    C::pick('p9_10', c='c9_10', a=154, b=true),
    C::pick('p9_11', b=false, a=155, c='c9_11'),
    C::pick('p9_12', a=156, b=true, c='c9_12'),
    C::pick('p9_13', b=false, c='c9_13', a=157),
    C::pick('p9_14', c='c9_14', a=158, b=true),
    C::pick('p9_15', b=false, a=159, c='c9_15'),
  ];
}

function group10(): vec<string> {
  return vec[
    C::pick('p10_0', a=160, b=true, c='c10_0'),
    C::pick('p10_1', b=false, c='c10_1', a=161),
    C::pick('p10_2', c='c10_2', a=162, b=true),
    C::pick('p10_3', b=false, a=163, c='c10_3'),
    C::pick('p10_4', a=164, b=true, c='c10_4'),
    C::pick('p10_5', b=false, c='c10_5', a=165),
    C::pick('p10_6', c='c10_6', a=166, b=true),
    C::pick('p10_7', b=false, a=167, c='c10_7'),
    C::pick('p10_8', a=168, b=true, c='c10_8'),
    C::pick('p10_9', b=false, c='c10_9', a=169),
    C::pick('p10_10', c='c10_10', a=170, b=true),
    C::pick('p10_11', b=false, a=171, c='c10_11'),
    C::pick('p10_12', a=172, b=true, c='c10_12'),
    C::pick('p10_13', b=false, c='c10_13', a=173),
    C::pick('p10_14', c='c10_14', a=174, b=true),
    C::pick('p10_15', b=false, a=175, c='c10_15'),
  ];
}

function group11(): vec<string> {
  return vec[
    C::pick('p11_0', a=176, b=true, c='c11_0'),
    C::pick('p11_1', b=false, c='c11_1', a=177),
    C::pick('p11_2', c='c11_2', a=178, b=true),
    C::pick('p11_3', b=false, a=179, c='c11_3'),
    C::pick('p11_4', a=180, b=true, c='c11_4'),
    C::pick('p11_5', b=false, c='c11_5', a=181),
    C::pick('p11_6', c='c11_6', a=182, b=true),
    C::pick('p11_7', b=false, a=183, c='c11_7'),
    C::pick('p11_8', a=184, b=true, c='c11_8'),
    C::pick('p11_9', b=false, c='c11_9', a=185),
    C::pick('p11_10', c='c11_10', a=186, b=true),
    C::pick('p11_11', b=false, a=187, c='c11_11'),
    C::pick('p11_12', a=188, b=true, c='c11_12'),
    C::pick('p11_13', b=false, c='c11_13', a=189),
    C::pick('p11_14', c='c11_14', a=190, b=true),
    C::pick('p11_15', b=false, a=191, c='c11_15'),
  ];
}

function group12(): vec<string> {
  return vec[
    C::pick('p12_0', a=192, b=true, c='c12_0'),
    C::pick('p12_1', b=false, c='c12_1', a=193),
    C::pick('p12_2', c='c12_2', a=194, b=true),
    C::pick('p12_3', b=false, a=195, c='c12_3'),
    C::pick('p12_4', a=196, b=true, c='c12_4'),
    C::pick('p12_5', b=false, c='c12_5', a=197),
    C::pick('p12_6', c='c12_6', a=198, b=true),
    C::pick('p12_7', b=false, a=199, c='c12_7'),
    C::pick('p12_8', a=200, b=true, c='c12_8'),
    C::pick('p12_9', b=false, c='c12_9', a=201),
    C::pick('p12_10', c='c12_10', a=202, b=true),
    C::pick('p12_11', b=false, a=203, c='c12_11'),
    C::pick('p12_12', a=204, b=true, c='c12_12'),
    C::pick('p12_13', b=false, c='c12_13', a=205),
    C::pick('p12_14', c='c12_14', a=206, b=true),
    C::pick('p12_15', b=false, a=207, c='c12_15'),
  ];
}

function group13(): vec<string> {
  return vec[
    C::pick('p13_0', a=208, b=true, c='c13_0'),
    C::pick('p13_1', b=false, c='c13_1', a=209),
    C::pick('p13_2', c='c13_2', a=210, b=true),
    C::pick('p13_3', b=false, a=211, c='c13_3'),
    C::pick('p13_4', a=212, b=true, c='c13_4'),
    C::pick('p13_5', b=false, c='c13_5', a=213),
    C::pick('p13_6', c='c13_6', a=214, b=true),
    C::pick('p13_7', b=false, a=215, c='c13_7'),
    C::pick('p13_8', a=216, b=true, c='c13_8'),
    C::pick('p13_9', b=false, c='c13_9', a=217),
    C::pick('p13_10', c='c13_10', a=218, b=true),
    C::pick('p13_11', b=false, a=219, c='c13_11'),
    C::pick('p13_12', a=220, b=true, c='c13_12'),
    C::pick('p13_13', b=false, c='c13_13', a=221),
    C::pick('p13_14', c='c13_14', a=222, b=true),
    C::pick('p13_15', b=false, a=223, c='c13_15'),
  ];
}

function group14(): vec<string> {
  return vec[
    C::pick('p14_0', a=224, b=true, c='c14_0'),
    C::pick('p14_1', b=false, c='c14_1', a=225),
    C::pick('p14_2', c='c14_2', a=226, b=true),
    C::pick('p14_3', b=false, a=227, c='c14_3'),
    C::pick('p14_4', a=228, b=true, c='c14_4'),
    C::pick('p14_5', b=false, c='c14_5', a=229),
    C::pick('p14_6', c='c14_6', a=230, b=true),
    C::pick('p14_7', b=false, a=231, c='c14_7'),
    C::pick('p14_8', a=232, b=true, c='c14_8'),
    C::pick('p14_9', b=false, c='c14_9', a=233),
    C::pick('p14_10', c='c14_10', a=234, b=true),
    C::pick('p14_11', b=false, a=235, c='c14_11'),
    C::pick('p14_12', a=236, b=true, c='c14_12'),
    C::pick('p14_13', b=false, c='c14_13', a=237),
    C::pick('p14_14', c='c14_14', a=238, b=true),
    C::pick('p14_15', b=false, a=239, c='c14_15'),
  ];
}

function group15(): vec<string> {
  return vec[
    C::pick('p15_0', a=240, b=true, c='c15_0'),
    C::pick('p15_1', b=false, c='c15_1', a=241),
    C::pick('p15_2', c='c15_2', a=242, b=true),
    C::pick('p15_3', b=false, a=243, c='c15_3'),
    C::pick('p15_4', a=244, b=true, c='c15_4'),
    C::pick('p15_5', b=false, c='c15_5', a=245),
    C::pick('p15_6', c='c15_6', a=246, b=true),
    C::pick('p15_7', b=false, a=247, c='c15_7'),
    C::pick('p15_8', a=248, b=true, c='c15_8'),
    C::pick('p15_9', b=false, c='c15_9', a=249),
    C::pick('p15_10', c='c15_10', a=250, b=true),
    C::pick('p15_11', b=false, a=251, c='c15_11'),
    C::pick('p15_12', a=252, b=true, c='c15_12'),
    C::pick('p15_13', b=false, c='c15_13', a=253),
    C::pick('p15_14', c='c15_14', a=254, b=true),
    C::pick('p15_15', b=false, a=255, c='c15_15'),
  ];
}

function groups(): vec<vec<string>> {
  return vec[
    group0(),
    group1(),
    group2(),
    group3(),
    group4(),
    group5(),
    group6(),
    group7(),
    group8(),
    group9(),
    group10(),
    group11(),
    group12(),
    group13(),
    group14(),
    group15(),
  ];
}

<<__EntryPoint>>
function main(): void {
  foreach (groups() as $group) {
    echo implode(' ', $group)."\n";
  }
}
