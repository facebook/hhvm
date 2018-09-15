(** [cppo_rules extension] will add rules to Ocamlbuild so that
    cppo is applied to files ending in "cppo.[extension]".

    By default rules are inserted for files ending with "ml", "mli" and
    "mlpack". *)
val cppo_rules : string -> unit

val dispatcher : Ocamlbuild_plugin.hook -> unit
