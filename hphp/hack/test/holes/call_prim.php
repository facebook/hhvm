<?hh

function prim(int $i): void {}

function call_prim(float $f, ?int $x): void {
    /* HH_FIXME[4110] */
    prim($f);

    /* HH_FIXME[4110] */
    prim($x);
}

function call_prim_cast(float $f, ?int $x): void {
    /* HH_FIXME[4417] */
    prim(unsafe_cast<float,int>($f));

    /* HH_FIXME[4417] */
    prim(unsafe_cast<?int,int>($x));
}
