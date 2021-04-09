<?hh

function prim(int $i): void {}

function call_prim(float $f, ?int $x): void {
    /* HH_FIXME[4110] */
    prim($f);

    /* HH_FIXME[4110] */
    prim($x);
}
