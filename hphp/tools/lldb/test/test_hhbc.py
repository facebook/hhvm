# Copyright 2022-present Facebook. All Rights Reserved.

# pyre-unsafe

from . import base  # usort: skip (must be first, needed for sys.path side-effects)
import hphp.tools.lldb.hhbc as hhbc
import hphp.tools.lldb.utils as utils


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
            "+38: IterInit WithKeys 0 L:0 125",
            "+46: IterGetValue WithKeys 0 L:0",
            "+50: PopL L:1",
            "+52: IterGetKey WithKeys 0 L:0",
            "+56: PopL L:2",
            "+58: String 6",  # 6=>"\n"
            "+63: CGetL2 L:1:1",
            "+66: Concat",
            "+67: Print",
            "+68: PopC",
            "+69: String 6",  # 6=>"\n"'
            "+74: CGetL2 L:2:2",
            "+77: Concat",
            "+78: Print",
            "+79: PopC",
            "+80: IterInit WithKeys 1 L:0 67",
            "+88: IterGetValue WithKeys 1 L:0",
            "+92: PopL L:3",
            "+94: IterGetKey WithKeys 1 L:0",
            "+98: PopL L:4",
            "+100: String 6",  # 6=>"\n"
            "+105: CGetL2 L:3:3",
            "+108: Concat",
            "+109: Print",
            "+110: PopC",
            "+111: String 6",  # 6=>"\n"
            "+116: CGetL2 L:4:4",
            "+119: Concat",
            "+120: Print",
            "+121: PopC",
            "+122: IterFree 1",
            "+124: IterFree 0",
            "+126: Jmp 37",
            "+131: IterNext WithKeys 1 L:0 -43",
            "+139: Jmp 8",
            "+144: IterFree 1",
            "+146: Throw",
            "+147: IterNext WithKeys 0 L:0 -101",
            "+155: Jmp 8",
            "+160: IterFree 0",
            "+162: Throw",
            "+163: Int 1",
            "+172: RetC",
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
            "+8: IterInit 0 L:0 111",
            "+16: IterGetValue 0 L:0",
            "+20: PopL L:1",
            "+22: CGetL L:1:1",
            "+25: Switch 0 0 <31 41 51 61 71>",  # 0=>Unbounded
            "+56: String 0",  # 0=>\"label_0\n\"
            "+61: Jmp 40",
            "+66: String 1",  # 1=>\"label_1\n\"
            "+71: Jmp 30",
            "+76: String 2",  # 2=>\"label_2\n\"
            "+81: Jmp 20",
            "+86: String 3",  # 3=>\"label_3\n\"
            "+91: Jmp 10",
            "+96: String 4",  # 4=>\"label_4\n\"
            "+101: Print",
            "+102: PopC",
            "+103: IterNext 0 L:0 -87",
            "+111: Jmp 8",
            "+116: IterFree 0",
            "+118: Throw",
            "+119: Int 1",
            "+128: RetC",
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
