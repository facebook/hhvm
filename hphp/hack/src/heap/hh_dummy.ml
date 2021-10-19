(*
 * From ocaml 4.12, libraries with any .ml modules will no longer
 * generates libfoo.a files.
 * Buck v1 is still expecting these, so the easiest workaround until
 * Buck v2 is to provide an empty module to trigger the generation
 * of libfoo.a
 *)
let () = ()
