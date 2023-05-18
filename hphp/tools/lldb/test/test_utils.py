# Copyright 2022-present Facebook. All Rights Reserved.

from . import base

import hhvm_lldb.utils as utils

class UtilsGivenTargetTestCase(base.TestHHVMBinary):
    def setUp(self):
        super().setUp() # No Hack file needed; just load HHVM target

    def test_Type(self):
        # Make sure some base HPHP types can be found by the debugger
        main_types = [
            "HPHP::ActRec",
            "HPHP::ArrayData",
            "HPHP::AsioBlockable",
            #"HPHP::Class",  # "LLDB currently crashes when looking up HPHP::Class type"
            "HPHP::DataType",
            "HPHP::Func",
            "HPHP::FuncId",
            "HPHP::Object",
            "HPHP::ObjectData",
            "HPHP::String",
            "HPHP::TypedValue",
            "HPHP::UnalignedTypedValue",
            "HPHP::Value",
        ]
        for name in main_types:
            with self.subTest(name=name):
                typ = utils.Type(name, self.target)
                self.assertTrue(typ.IsValid())

    def test_Global(self):
        g_code = utils.Global("HPHP::jit::tc::g_code", self.target)
        self.assertTrue(g_code.IsValid())
        self.assertTrue(g_code.type.IsPointerType())

    def test_Enum(self):
        k_vec_kind = utils.Enum("HPHP::ArrayData::ArrayKind", "kVecKind", self.target)
        self.assertTrue(k_vec_kind.IsValid())
        self.assertTrue(k_vec_kind.GetName(), "kVecKind")

    def test_get(self):
        g_code = utils.Global("HPHP::jit::tc::g_code", self.target)
        code_size = utils.get(g_code, "m_threadLocalStart")
        self.assertTrue(code_size.IsValid())

    def test_rawtype_remove_typedef(self):
        ty = utils.Type("HPHP::Native::NativeDataInfo::InitFunc", self.target)
        self.assertTrue(ty.IsTypedefType())
        raw = utils.rawtype(ty)
        self.assertFalse(raw.IsTypedefType())
        self.assertEqual(raw.name, "void (*)(HPHP::ObjectData *)")

    def test_template_type(self):
        ty = utils.Type("HPHP::VMFixedVector<ObjectProps::quick_index>", self.target)
        self.assertTrue(ty.IsTypedefType())
        template = utils.template_type(ty)
        self.assertEqual(template, "HPHP::FixedVector")


class UtilsGivenFrameTestCase(base.TestHHVMBinary):
    def setUp(self):
        super().setUp(test_file="quick/method2.php", interp = True)

    def test_nameof_func(self):
        self.run_commands(["b lookupObjMethod", "continue", "thread step-out"])
        func = self.frame.FindVariable("func")
        name = utils.nameof(func)
        self.assertEqual(name, "B::foo")

    def test_nameof_class(self):
        self.run_until_breakpoint("lookupObjMethod")
        cls = self.frame.FindVariable("cls")
        name = utils.nameof(cls)
        self.assertEqual(name, "B")

    def test_rawtype_remove_const(self):
        self.run_until_breakpoint("lookupObjMethod")
        cls = self.frame.FindVariable("cls")
        self.assertEqual(cls.type.name, "const HPHP::Class *")
        raw_type = utils.rawtype(cls.type)
        self.assertEqual(raw_type.name, "HPHP::Class *")

    def test_nameof_objectdata(self):
        self.run_commands(["b newObjImpl", "continue", "thread step-out"])
        od = self.thread.return_value
        name = utils.nameof(od)
        self.assertEqual(name, "B")

    def test_rawptr(self):
        self.run_commands(["b lookupObjMethod", "continue", "thread step-out"])
        func = self.frame.FindVariable("func")
        smart_ptr = func.GetChildMemberWithName("m_fullName")
        raw_ptr = utils.rawptr(smart_ptr)
        self.assertTrue(raw_ptr.type.IsPointerType())
        self.assertEqual(raw_ptr.unsigned, 1)

    def test_rawptr_std_unique_ptr(self):
        self.run_until_breakpoint("lookupObjMethod")
        s_func_vec = self.target.FindFirstGlobalVariable("HPHP::Func::s_funcVec")
        if not s_func_vec.IsValid():
            # lowptr builds don't have a funcVec
            return
        smart_ptr = utils.get(s_func_vec, "m_vals")
        self.assertEqual(utils.template_type(smart_ptr.type), "std::unique_ptr")
        raw_ptr = utils.rawptr(smart_ptr)
        self.assertEqual(utils.template_type(raw_ptr.type), "HPHP::detail::LowPtrImpl")

    def test_arch_regs(self):
        # Make sure we're consistent with what LLDB is telling us are FP, PC, SP,
        # for both x86 and ARM
        fp = utils.reg('fp', self.frame)
        self.assertEqual(fp.unsigned, self.frame.fp)
        sp = utils.reg('sp', self.frame)
        self.assertEqual(sp.unsigned, self.frame.sp)
        ip = utils.reg('ip', self.frame)
        self.assertEqual(ip.unsigned, self.frame.pc)
