<?hh
// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

final class C {
  public function __construct(private dict<string, mixed> $spec) {}
}

function foo(dict<string, mixed> $xs): void {
  $x = idx($xs, 'whatever');
  if (HH\is_any_array($x)) {
    /* HH_FIXME[4110] */
    new C(dict($x));
  }
}

function bar(dict<string, mixed> $xs): void {
  $x = idx($xs, 'whatever');
  if (HH\is_any_array($x)) {
    $y = dict($x);
    /* HH_FIXME[4110] */
    new C($y);
  }
}
