<?hh
<<file: __EnableUnstableFeatures('open_tuples', 'type_splat')>>
function expect_int(int $i): void {}

function call_with_open_tuple((float, optional int, nonnull...) $t): void {
    /* HH_FIXME[4110] */
    expect_int($t);
}
function call_with_splat_tuple<T as (mixed...)>((int, ...T) $args): void {
    /* HH_FIXME[4110] */
    expect_int($args);
}
