# Varray & Darray

`varray`, `darray`, and `varray_or_darray` are legacy types that are usually equivalent to
`vec`, `dict`, and `vec_or_dict` respectively.
They should *never* be used in new code.

In positions where a type may be observable as a `TypeStructure`, these types will result in different `TypeStructureKind` values.
This is the only direct effect, but user-defined uses of `TypeStructure` values may interpret these kinds differently from their `vec`/`dict`/`vec_or_dict` counterparts (this is not recommended).

In all other positions, it is equivalent and recommended to use their `vec`/`dict`/`vec_or_dict` counterparts instead.
