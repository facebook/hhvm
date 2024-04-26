namespace HPHP::hackc {

struct HhbcFlags {
  bool ltr_assign;
  bool uvs;
  bool log_extern_compiler_perf;
  bool enable_intrinsics_extension;
  bool emit_cls_meth_pointers;
  bool fold_lazy_class_keys;
  bool optimize_reified_param_checks;
  bool stress_shallow_decl_deps;
  bool stress_folded_decl_deps;
  bool enable_native_enum_class_labels;
  bool optimize_param_lifetimes;
  bool optimize_local_lifetimes;
  bool optimize_local_iterators;
  bool optimize_is_type_checks;
};

} // namespace HPHP::hackc
