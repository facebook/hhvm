#
# Autogenerated by Thrift for thrift/compiler/test/fixtures/interactions/src/module.thrift
#
# DO NOT EDIT UNLESS YOU ARE SURE THAT YOU KNOW WHAT YOU ARE DOING
#  @generated
#


cdef extern from "thrift/compiler/test/fixtures/interactions/gen-cpp2/module_handlers.h" namespace "::cpp2":
    cdef cppclass cMyServiceSvIf "::cpp2::MyServiceSvIf":
        pass

cdef extern from "thrift/compiler/test/fixtures/interactions/gen-cpp2/module_handlers.h" namespace "::cpp2":
    cdef cppclass cFactoriesSvIf "::cpp2::FactoriesSvIf":
        pass

cdef extern from "thrift/compiler/test/fixtures/interactions/gen-cpp2/module_handlers.h" namespace "::cpp2":
    cdef cppclass cPerformSvIf "::cpp2::PerformSvIf":
        pass

cdef extern from "thrift/compiler/test/fixtures/interactions/gen-cpp2/module_handlers.h" namespace "::cpp2":
    cdef cppclass cInteractWithSharedSvIf "::cpp2::InteractWithSharedSvIf":
        pass

cdef extern from "thrift/compiler/test/fixtures/interactions/gen-cpp2/module_handlers.h" namespace "::cpp2":
    cdef cppclass cBoxServiceSvIf "::cpp2::BoxServiceSvIf":
        pass
