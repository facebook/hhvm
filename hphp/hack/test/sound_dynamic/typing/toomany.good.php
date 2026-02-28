<?hh
// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

<<__SupportDynamicType>>
function foo(
    HH\FormatString<PlainSprintf> $message,
    supportdyn<mixed> ...$args
  ): void {
  }

<<__SupportDynamicType>>
function bar(
    int $i,
    vec<string> $vs,
    string $s,
  ): void {
    foo(
        '$vs[0] = %s, $i = %20d, $s = %s',
        $vs[0],
        $i,
        $s,
      );
    }
