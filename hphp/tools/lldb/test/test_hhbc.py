# Copyright 2022-present Facebook. All Rights Reserved.

from . import base

import hhvm_lldb.utils as utils
import hhvm_lldb.hhbc as hhbc

# Only setting allocateBCRegion breakpoint
# after we've inited the process and updated
# constants (because allocateBCRegion is called
# many times before then for bytecode we don't care about).
commands_to_get_to_bc_region = [
    "breakpoint set -M hphp_process_init",
    "continue",
    "breakpoint delete -f",
    "breakpoint set -M update_constants_and_options",
    "continue",
    "breakpoint delete -f",
    "breakpoint set -M allocateBCRegion",
    "continue",
]

class HHBCTestCase(base.TestHHVMBinary):
    def setUp(self):
        super().setUp(test_file="quick/asm_iterbreak.hhas", interp=True, allow_hhas=True)

    def test_hhx(self):
        self.run_commands(commands_to_get_to_bc_region)
        _, output = self.run_commands(["hhx bc bc+bclen"])
        # TODO replace Ids in 'String <Id>' with actual string value when lookup_litstr works
        expected_bcs = [
            '+0: NewDictArray 3',
            '+2: String 0',  # 0=>"hello"
            '+7: String 1',  # 1=>"world"
            '+12: AddElemC',
            '+13: String 2',  # 2=>"this"
            '+18: String 3',  # 3=>"is"
            '+23: AddElemC',
            '+24: String 4',  # 4=>"a"
            '+29: String 5',  # 5=>"test"
            '+34: AddElemC',
            '+35: SetL L:0',
            '+37: PopC',
            '+38: CGetL L:0:0',
            '+41: IterInit 0 K:2 V:1 108',
            '+50: String 6',  # 6=>"\n"
            '+55: CGetL2 L:1:1',
            '+58: Concat',
            '+59: Print',
            '+60: PopC',
            '+61: String 6',  # 6=>"\n"'
            '+66: CGetL2 L:2:2',
            '+69: Concat',
            '+70: Print',
            '+71: PopC',
            '+72: CGetL L:0:0',
            '+75: IterInit 1 K:4 V:3 57',
            '+84: String 6',  # 6=>"\n"
            '+89: CGetL2 L:3:3',
            '+92: Concat',
            '+93: Print',
            '+94: PopC',
            '+95: String 6',  # 6=>"\n"
            '+100: CGetL2 L:4:4',
            '+103: Concat',
            '+104: Print',
            '+105: PopC',
            '+106: IterFree 1',
            '+108: IterFree 0',
            '+110: Jmp 39',
            '+115: IterNext 1 K:4 V:3 -31',
            '+124: Jmp 8',
            '+129: IterFree 1',
            '+131: Throw',
            '+132: IterNext 0 K:2 V:1 -82',
            '+141: Jmp 8',
            '+146: IterFree 0',
            '+148: Throw',
            '+149: Int 1',
            '+158: RetC',
        ]
        output_lines = output.strip().split("\n")
        self.assertEqual(len(output_lines), len(expected_bcs))
        for line, expected_bc in zip(output_lines, expected_bcs):
            line = '+' + line.split("+")[1]
            self.assertEqual(line, expected_bc)


    def test_helper_functions(self):
        op_name_to_immeds = {
            "Nop": [],
            "PopL": ["LA"],
            "Int": ["I64A"],
            "ClsCnsD": ["SA", "SA"],
            "CGetL": ["NLA"],
            "FCallClsMethodD": ["FCA", "SA", "SA"],
            "FCallObjMethodD": ["FCA", "SA", "OA", "SA"],
            "SetOpM": ["IVA", "OA", "KA"],
            "MemoSetEager": ["LAR"],
            "NewDictArray": ["IVA"],
        }
        for op_name, immeds in op_name_to_immeds.items():
            op = utils.Enum("HPHP::Op", op_name, self.target)

            with self.subTest(f"HHBC.num_imms({op_name})"):
                num_imms = hhbc.HHBC.num_imms(op, self.target)
                self.assertEqual(num_imms, len(immeds))

            with self.subTest(f"HHBC.op_name({op_name})"):
                name = hhbc.HHBC.op_name(op, self.target)
                self.assertEqual(name, op_name)

            with self.subTest(f"HHBC.imm_type({op_name})"):
                for ix, immed in enumerate(immeds):
                    immtype = hhbc.HHBC.imm_type(op, ix, self.target)
                    self.assertEqual(immtype.value, immed)

        # For the following, just verifying we can look up the types/enums
        # successfully, i.e. the calls don't fail
        with self.subTest("HHBC.iva_imm_types"):
            imm_types = hhbc.iva_imm_types(self.target)
            self.assertEqual(len(imm_types), 4)

        with self.subTest("HHBC.vec_imm_types"):
            imm_types = hhbc.vec_imm_types(self.target)
            self.assertEqual(len(imm_types), 3)

        with self.subTest("HHBC.vec_elm_sizes"):
            elm_sizes = hhbc.vec_elm_sizes(self.target)
            self.assertEqual(len(elm_sizes), 3)

        with self.subTest("HHBC.tv_iva_mcodes"):
            mcodes = hhbc.tv_iva_mcodes(self.target)
            self.assertEqual(len(mcodes), 2)

        with self.subTest("HHBC.tv_loc_mcodes"):
            mcodes = hhbc.tv_loc_mcodes(self.target)
            self.assertEqual(len(mcodes), 2)

        with self.subTest("HHBC.str_imm_mcodes"):
            mcodes = hhbc.str_imm_mcodes(self.target)
            self.assertEqual(len(mcodes), 3)

        with self.subTest("HHBC.rata_arrs"):
            arrs = hhbc.rata_arrs(self.target)
            self.assertEqual(len(arrs), 12)

        with self.subTest("HHBC.rata_objs"):
            objs = hhbc.rata_objs(self.target)
            self.assertEqual(len(objs), 8)


class HHBCTestCase2(base.TestHHVMBinary):
    def setUp(self):
        super().setUp(test_file="quick/asm_array.hhas", interp=True, allow_hhas=True)

    def test_hhx(self):
        self.run_commands(commands_to_get_to_bc_region)
        _, output = self.run_commands(["hhx bc bc+bclen"])
        # TODO replace Ids in 'String <Id>' with actual string value when lookup_litstr works
        # The sub-opcodes, like Any and Warn, may also be able to be substituted back in the future.
        expected_bcs = [
            "+0: NullUninit",
            "+1: NullUninit",
            "+2: String 0",  # 0=>"arr"
            "+7: String 1",  # 1=>"ClassWithArray"
            "+12: ClassGetC 0", # 0=>"Normal"
            "+14: CGetS 0",  # 0=>Any (a subop)
            "+16: FCallFuncD <> 1 1 \"\" \"\" - \"\" 2",  # 2=>"var_dump"
            "+23: PopC",
            "+24: String 1",  # 1=>"ClassWithArray"
            "+29: SetL L:0",
            "+31: PopC",
            "+32: NullUninit",
            "+33: NullUninit",
            "+34: String 0",  # 0=>"arr"
            "+39: Int 2",
            "+48: CGetL L:0:0",
            "+51: ClassGetC 0", # 0=>"Normal"
            "+53: BaseSC 2 0 1 0",  # 1=>Warn 0=>Any
            "+58: QueryM 3 0 EC:1 Any",  # 0=>CGet
            "+64: FCallFuncD <> 1 1 \"\" \"\" - \"\" 2",  # 2=>"var_dump"
            "+71: PopC",
            "+72: True",
            "+73: RetC",
        ]

        output_lines = output.strip().split("\n")
        self.assertEqual(len(output_lines), len(expected_bcs))
        for line, expected_bc in zip(output_lines, expected_bcs):
            line = '+' + line.split("+")[1]
            self.assertEqual(line, expected_bc)


class HHBCTestCase3(base.TestHHVMBinary):
    def setUp(self):
        super().setUp(test_file="quick/asm_switch.hhas", interp=True, allow_hhas=True)

    def test_hhx(self):
        self.run_commands(commands_to_get_to_bc_region)
        _, output = self.run_commands(["hhx bc bc+bclen"])
        # TODO replace Ids in 'String <Id>' with actual string value when lookup_litstr works
        expected_bcs = [
            "+0: Dict 0", # 0=>dict(0=>0,1=>1,2=>2,3=>3,4=>4)",
            "+5: SetL L:0",
            "+7: PopC",
            "+8: CGetL L:0:0",
            "+11: IterInit 0 NK V:1 107",
            "+20: CGetL L:1:1",
            "+23: Switch 0 0 <31 41 51 61 71>",  # 0=>Unbounded
            "+54: String 0", # 0=>\"label_0\n\"
            "+59: Jmp 40",
            "+64: String 1",  #1=>\"label_1\n\"
            "+69: Jmp 30",
            "+74: String 2",  #2=>\"label_2\n\"
            "+79: Jmp 20",
            "+84: String 3",  #3=>\"label_3\n\"
            "+89: Jmp 10",
            "+94: String 4",  #4=>\"label_4\n\"
            "+99: Print",
            "+100: PopC",
            "+101: IterNext 0 NK V:1 -81",
            "+110: Jmp 8",
            "+115: IterFree 0",
            "+117: Throw",
            "+118: Int 1",
            "+127: RetC",
        ]

        output_lines = output.strip().split("\n")
        self.assertEqual(len(output_lines), len(expected_bcs))
        for line, expected_bc in zip(output_lines, expected_bcs):
            line = '+' + line.split("+")[1]
            self.assertEqual(line, expected_bc)


class HHBCTestCase4(base.TestHHVMBinary):
    def setUp(self):
        super().setUp(test_file="quick/asm_assert_t.hhas", interp=True, allow_hhas=True)

    def test_hhx(self):
        self.run_commands(commands_to_get_to_bc_region)
        self.run_commands(["continue"])  # to get to obj_type() function's bytecode
        _, output = self.run_commands(["hhx bc bc+bclen"])
        expected_bcs = [
            "+0: CGetL L:0:0",
            "+3: AssertRATStk 0 Obj",
            "+6: AssertRATL L:0 Obj",
            "+9: AssertRATL L:1 Obj",
            "+12: PopC",
            "+13: AssertRATL L:0 Obj",
            "+16: AssertRATL L:0 Obj",
            "+19: AssertRATL L:1 Obj",
            "+22: AssertRATL L:1 Obj",
            "+25: AssertRATL L:1 Obj",
            "+28: Int 1",
            "+37: RetC",
        ]

        output_lines = output.strip().split("\n")
        self.assertEqual(len(output_lines), len(expected_bcs))
        for line, expected_bc in zip(output_lines, expected_bcs):
            line = '+' + line.split("+")[1]
            self.assertEqual(line, expected_bc)
