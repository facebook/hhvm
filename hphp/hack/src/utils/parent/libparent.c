#define CAML_NAME_SPACE
#include <caml/mlvalues.h>
#include <caml/memory.h>

extern void exit_on_parent_exit_(int interval, int grace);

CAMLprim value exit_on_parent_exit(
        value ml_interval,
        value ml_grace
) {
    CAMLparam2(ml_interval, ml_grace);
    int interval = Int_val(ml_interval);
    int grace = Int_val(ml_grace);
    exit_on_parent_exit_(interval, grace);
    CAMLreturn(Val_unit);
}
