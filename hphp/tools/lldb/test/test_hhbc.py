# Copyright 2022-present Facebook. All Rights Reserved.

from . import base  # usort: skip (must be first, needed for sys.path side-effects)
import hhvm_lldb.hhbc as hhbc
import hhvm_lldb.utils as utils


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
        super().setUp(
            test_file="quick/asm_iterbreak.hhas", interp=True, allow_hhas=True
        )

    def test_hhx(self):
        self.run_commands(commands_to_get_to_bc_region)
        _, output = self.run_commands(["hhx bc bc+bclen"])
        # TODO replace Ids in 'String <Id>' with actual string value when lookup_litstr works
        expected_bcs = [
            "+0: NewDictArray 3",
            "+2: String 0",  # 0=>"hello"
            "+7: String 1",  # 1=>"world"
            "+12: AddElemC",
            "+13: String 2",  # 2=>"this"
            "+18: String 3",  # 3=>"is"
            "+23: AddElemC",
            "+24: String 4",  # 4=>"a"
            "+29: String 5",  # 5=>"test"
            "+34: AddElemC",
            "+35: SetL L:0",
            "+37: PopC",
            "+38: IterInit 0 K:2 V:1 L:0 109",
            "+48: String 6",  # 6=>"\n"
            "+53: CGetL2 L:1:1",
            "+56: Concat",
            "+57: Print",
            "+58: PopC",
            "+59: String 6",  # 6=>"\n"'
            "+64: CGetL2 L:2:2",
            "+67: Concat",
            "+68: Print",
            "+69: PopC",
            "+70: IterInit 1 K:4 V:3 L:0 59",
            "+80: String 6",  # 6=>"\n"
            "+85: CGetL2 L:3:3",
            "+88: Concat",
            "+89: Print",
            "+90: PopC",
            "+91: String 6",  # 6=>"\n"
            "+96: CGetL2 L:4:4",
            "+99: Concat",
            "+100: Print",
            "+101: PopC",
            "+102: IterFree 1",
            "+104: IterFree 0",
            "+106: Jmp 41",
            "+111: IterNext 1 K:4 V:3 L:0 -31",
            "+121: Jmp 8",
            "+126: IterFree 1",
            "+128: Throw",
            "+129: IterNext 0 K:2 V:1 L:0 -81",
            "+139: Jmp 8",
            "+144: IterFree 0",
            "+146: Throw",
            "+147: Int 1",
            "+156: RetC",
        ]
        output_lines = output.strip().split("\n")
        self.assertEqual(len(output_lines), len(expected_bcs))
        for line, expected_bc in zip(output_lines, expected_bcs):
            line = "+" + line.split("+")[1]
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
            "+12: ClassGetC 0",  # 0=>"Normal"
            "+14: CGetS 0",  # 0=>Any (a subop)
            '+16: FCallFuncD <> 1 1 "" "" - "" 2',  # 2=>"var_dump"
            "+23: PopC",
            "+24: String 1",  # 1=>"ClassWithArray"
            "+29: SetL L:0",
            "+31: PopC",
            "+32: NullUninit",
            "+33: NullUninit",
            "+34: String 0",  # 0=>"arr"
            "+39: Int 2",
            "+48: CGetL L:0:0",
            "+51: ClassGetC 0",  # 0=>"Normal"
            "+53: BaseSC 2 0 1 0",  # 1=>Warn 0=>Any
            "+58: QueryM 3 0 EC:1 Any",  # 0=>CGet
            '+64: FCallFuncD <> 1 1 "" "" - "" 2',  # 2=>"var_dump"
            "+71: PopC",
            "+72: True",
            "+73: RetC",
        ]

        output_lines = output.strip().split("\n")
        self.assertEqual(len(output_lines), len(expected_bcs))
        for line, expected_bc in zip(output_lines, expected_bcs):
            line = "+" + line.split("+")[1]
            self.assertEqual(line, expected_bc)


class HHBCTestCase3(base.TestHHVMBinary):
    def setUp(self):
        super().setUp(test_file="quick/asm_switch.hhas", interp=True, allow_hhas=True)

    def test_hhx(self):
        self.run_commands(commands_to_get_to_bc_region)
        _, output = self.run_commands(["hhx bc bc+bclen"])
        # TODO replace Ids in 'String <Id>' with actual string value when lookup_litstr works
        expected_bcs = [
            "+0: Dict 0",  # 0=>dict(0=>0,1=>1,2=>2,3=>3,4=>4)",
            "+5: SetL L:0",
            "+7: PopC",
            "+8: IterInit 0 NK V:1 L:0 109",
            "+18: CGetL L:1:1",
            "+21: Switch 0 0 <31 41 51 61 71>",  # 0=>Unbounded
            "+52: String 0",  # 0=>\"label_0\n\"
            "+57: Jmp 40",
            "+62: String 1",  # 1=>\"label_1\n\"
            "+67: Jmp 30",
            "+72: String 2",  # 2=>\"label_2\n\"
            "+77: Jmp 20",
            "+82: String 3",  # 3=>\"label_3\n\"
            "+87: Jmp 10",
            "+92: String 4",  # 4=>\"label_4\n\"
            "+97: Print",
            "+98: PopC",
            "+99: IterNext 0 NK V:1 L:0 -81",
            "+109: Jmp 8",
            "+114: IterFree 0",
            "+116: Throw",
            "+117: Int 1",
            "+126: RetC",
        ]

        output_lines = output.strip().split("\n")
        self.assertEqual(len(output_lines), len(expected_bcs))
        for line, expected_bc in zip(output_lines, expected_bcs):
            line = "+" + line.split("+")[1]
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
            line = "+" + line.split("+")[1]
            self.assertEqual(line, expected_bc)
