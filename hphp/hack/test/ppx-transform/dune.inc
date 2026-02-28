; GENERATED RULES

(rule
 (targets test_constant_ctor_embed_error.actual.ml)
 (deps
  (:pp pp.exe)
  (:input test_constant_ctor_embed_error.ml))
 (action
   (progn
     (with-stdout-to %{targets} (run ./%{pp} --impl %{input} -o %{targets}))
     (bash "arc f %{targets} > /dev/null 2>&1")
   )
 )
)
(rule
 (alias runtest)
 (action
  (diff test_constant_ctor_embed_error.expected.ml test_constant_ctor_embed_error.actual.ml)))

(rule
 (targets test_inline_record_embed_error.actual.ml)
 (deps
  (:pp pp.exe)
  (:input test_inline_record_embed_error.ml))
 (action
   (progn
     (with-stdout-to %{targets} (run ./%{pp} --impl %{input} -o %{targets}))
     (bash "arc f %{targets} > /dev/null 2>&1")
   )
 )
)
(rule
 (alias runtest)
 (action
  (diff test_inline_record_embed_error.expected.ml test_inline_record_embed_error.actual.ml)))
