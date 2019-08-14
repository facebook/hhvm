(library
  (name facts)
  (wrapped false)
  (modules
    facts
    facts_parser
    facts_smart_constructors
    flatten_smart_constructors
    rust_facts_ffi)
  (libraries
    core_kernel
    hhbc
    parser
    rust_facts_ffi
    (select rust_facts_ffi.ml from
      (facebook -> ffi/rust_facts_ffi.ml)
      (-> ffi/rust_facts_ffi_stubs.ml)
    )
    utils_hash)
  (preprocess (pps ppx_deriving.std)))
