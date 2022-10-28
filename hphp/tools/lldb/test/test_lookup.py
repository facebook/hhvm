from . import base

class LookupCommandTestCase(base.LLDBTestBase):

    def setUp(self):
        super().setUp(file="quick/method2.php", interp=True)

    def test_lookup_func_command(self):
        # Note: We can choose an earlier point to break;
        # just need to make sure the functions are loaded.
        self.run_until_breakpoint("lookupObjMethod")
        _, output = self.run_commands(["lookup func 0"])
        self.assertEqual("(HPHP::Func *) m_s = NULL", output.strip())
