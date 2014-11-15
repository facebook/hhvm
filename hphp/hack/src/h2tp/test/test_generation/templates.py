from __future__ import absolute_import
from __future__ import division
from __future__ import print_function
from __future__ import unicode_literals
from string import Template

FILE_STR = """
from converter_test_base import ConverterTestCase
import unittest

from convert_hhvm_collection_base import ConvertHHVMCollectionBase
import unittest
"""

CLASS_TMPL = Template("""
class ${name}(ConverterTestCase):
  def binary_path(self):
    return '${bin_path}'

  def binary_name(self):
    return '${bin_name}'

  def execution_prefix(self):
    return "$$GLOBALS['HACKLIB_ROOT'] = '${code_dir}/resources/hacklib.php';"

  def changeHH(self):
    return ${changeHH}

  def additional_options(self):
    return [${additional_opts}]
""")

SKIP_TMPL = Template("""
  @unittest.skip(\"""${reason}\""")
""")


FUNC_TMPL = Template("""
  def test_${name}(self):
    self.compare('${in_path}', '${out_path}', '${filename}')
    self.compare_execution_converted('${in_path}', '${out_path}')
""")

unparser_FUNC_TMPL = Template("""
  def test_${name}(self):
    self.compare('${in_path}', '${out_path}', '${filename}')
    self.compare_execution_unparsed('${in_path}', '${out_path}')
""")

HHVM_COLL_TEST_TMPL = Template("""
class ConvertHHVMCollectionTestsTestCase(ConvertHHVMCollectionBase):
  def test_hhvm_coll_test_inputs_can_be_converted(self):
    self.verify()

  def testsDir(self):
    return '${test_input_dir}'

  def binary_path(self):
    return '${bin_path}'

  def execution_prefix(self):
    return "$$GLOBALS['HACKLIB_ROOT'] = '${code_dir}/resources/hacklib.php';"

""")
