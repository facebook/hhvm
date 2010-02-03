ROOTDIR = tests/php-5.2.5
PHPT_FILES = \
        $(wildcard $(ROOTDIR)/ext/bcmath/tests/*.phpt) \
        $(wildcard $(ROOTDIR)/ext/hash/tests/*.phpt) \
        $(wildcard $(ROOTDIR)/ext/date/tests/*.phpt) \
	$(wildcard $(ROOTDIR)/ext/json/tests/*.phpt) \
	$(wildcard $(ROOTDIR)/sapi/cgi/tests/*.phpt) \
	$(wildcard $(ROOTDIR)/sapi/tests/*.phpt) \
	$(wildcard $(ROOTDIR)/sapi/cli/tests/*.phpt) \
	$(wildcard $(ROOTDIR)/tests/basic/*.phpt) \
	$(wildcard $(ROOTDIR)/tests/lang/*phpt) \
	$(wildcard $(ROOTDIR)/tests/strings/*phpt) \
	$(wildcard $(ROOTDIR)/tests/func/*.phpt) \
	$(wildcard $(ROOTDIR)/tests/run-test/*phpt) \
	$(wildcard $(ROOTDIR)/tests/classes/*.phpt) \
	$(wildcard $(ROOTDIR)/Zend/tests/*.phpt) \

# some of these are skipped for trivial message differences
# some are skipped because the functionality isn't implemented
SKIPPED = \
        tests/php-5.2.5/sapi/cgi/tests/001.phpt \
        tests/php-5.2.5/sapi/cgi/tests/002.phpt \
        tests/php-5.2.5/sapi/cgi/tests/003.phpt \
        tests/php-5.2.5/sapi/cgi/tests/004.phpt \
        tests/php-5.2.5/sapi/cgi/tests/005.phpt \
        tests/php-5.2.5/sapi/cgi/tests/006.phpt \
        tests/php-5.2.5/sapi/cgi/tests/007.phpt \
        tests/php-5.2.5/sapi/cgi/tests/008.phpt \
        tests/php-5.2.5/sapi/cgi/tests/009.phpt \
        tests/php-5.2.5/sapi/tests/test006.phpt \
        tests/php-5.2.5/sapi/tests/test007.phpt \
        tests/php-5.2.5/sapi/cli/tests/001.phpt \
        tests/php-5.2.5/sapi/cli/tests/002.phpt \
        tests/php-5.2.5/sapi/cli/tests/002-win32.phpt \
        tests/php-5.2.5/sapi/cli/tests/003-2.phpt \
        tests/php-5.2.5/sapi/cli/tests/003.phpt \
        tests/php-5.2.5/sapi/cli/tests/004.phpt \
        tests/php-5.2.5/sapi/cli/tests/005.phpt \
        tests/php-5.2.5/sapi/cli/tests/006.phpt \
        tests/php-5.2.5/sapi/cli/tests/007.phpt \
        tests/php-5.2.5/sapi/cli/tests/008.phpt \
        tests/php-5.2.5/sapi/cli/tests/009.phpt \
        tests/php-5.2.5/sapi/cli/tests/010-2.phpt \
        tests/php-5.2.5/sapi/cli/tests/010.phpt \
        tests/php-5.2.5/sapi/cli/tests/011.phpt \
        tests/php-5.2.5/sapi/cli/tests/012.phpt \
        tests/php-5.2.5/sapi/cli/tests/013.phpt \
        tests/php-5.2.5/sapi/cli/tests/014.phpt \
        tests/php-5.2.5/sapi/cli/tests/015.phpt \
        tests/php-5.2.5/tests/basic/002.phpt \
        tests/php-5.2.5/tests/basic/003.phpt \
        tests/php-5.2.5/tests/basic/004.phpt \
        tests/php-5.2.5/tests/basic/005.phpt \
        tests/php-5.2.5/tests/basic/011.phpt \
        tests/php-5.2.5/tests/basic/012.phpt \
        tests/php-5.2.5/tests/basic/013.phpt \
        tests/php-5.2.5/tests/basic/014.phpt \
        tests/php-5.2.5/tests/basic/015.phpt \
        tests/php-5.2.5/tests/basic/016.phpt \
        tests/php-5.2.5/tests/basic/017.phpt \
        tests/php-5.2.5/tests/basic/018.phpt \
        tests/php-5.2.5/tests/basic/019.phpt \
        tests/php-5.2.5/tests/basic/020.phpt \
        tests/php-5.2.5/tests/basic/021.phpt \
        tests/php-5.2.5/tests/basic/024.phpt \
        tests/php-5.2.5/tests/basic/025.phpt \
        tests/php-5.2.5/tests/basic/026.phpt \
        tests/php-5.2.5/tests/basic/027.phpt \
        tests/php-5.2.5/tests/basic/bug29971.phpt \
\
        tests/php-5.2.5/tests/run-test/test004.phpt \
        tests/php-5.2.5/tests/run-test/test006.phpt \
        tests/php-5.2.5/tests/run-test/test010.phpt \
\
        tests/php-5.2.5/tests/classes/__set__get_002.phpt \
        tests/php-5.2.5/tests/classes/__set__get_003.phpt \
\
        tests/php-5.2.5/Zend/tests/errmsg_001.phpt \
        tests/php-5.2.5/Zend/tests/errmsg_002.phpt \
        tests/php-5.2.5/Zend/tests/errmsg_003.phpt \
        tests/php-5.2.5/Zend/tests/errmsg_004.phpt \
        tests/php-5.2.5/Zend/tests/errmsg_005.phpt \
        tests/php-5.2.5/Zend/tests/errmsg_006.phpt \
        tests/php-5.2.5/Zend/tests/errmsg_007.phpt \
        tests/php-5.2.5/Zend/tests/errmsg_008.phpt \
        tests/php-5.2.5/Zend/tests/errmsg_009.phpt \
        tests/php-5.2.5/Zend/tests/errmsg_010.phpt \
        tests/php-5.2.5/Zend/tests/errmsg_011.phpt \
        tests/php-5.2.5/Zend/tests/errmsg_012.phpt \
        tests/php-5.2.5/Zend/tests/errmsg_013.phpt \
        tests/php-5.2.5/Zend/tests/errmsg_014.phpt \
        tests/php-5.2.5/Zend/tests/errmsg_015.phpt \
        tests/php-5.2.5/Zend/tests/errmsg_016.phpt \
        tests/php-5.2.5/Zend/tests/errmsg_017.phpt \
        tests/php-5.2.5/Zend/tests/errmsg_018.phpt \
        tests/php-5.2.5/Zend/tests/errmsg_019.phpt \
        tests/php-5.2.5/Zend/tests/errmsg_020.phpt \
        tests/php-5.2.5/Zend/tests/errmsg_022.phpt \
        tests/php-5.2.5/Zend/tests/errmsg_023.phpt \
        tests/php-5.2.5/Zend/tests/errmsg_024.phpt \
        tests/php-5.2.5/Zend/tests/errmsg_025.phpt \
        tests/php-5.2.5/Zend/tests/errmsg_026.phpt \
        tests/php-5.2.5/Zend/tests/errmsg_027.phpt \
        tests/php-5.2.5/Zend/tests/errmsg_028.phpt \
        tests/php-5.2.5/Zend/tests/errmsg_029.phpt \
        tests/php-5.2.5/Zend/tests/errmsg_030.phpt \
        tests/php-5.2.5/Zend/tests/errmsg_031.phpt \
        tests/php-5.2.5/Zend/tests/errmsg_032.phpt \
        tests/php-5.2.5/Zend/tests/errmsg_033.phpt \
        tests/php-5.2.5/Zend/tests/errmsg_034.phpt \
        tests/php-5.2.5/Zend/tests/errmsg_035.phpt \
        tests/php-5.2.5/Zend/tests/errmsg_036.phpt \
        tests/php-5.2.5/Zend/tests/errmsg_037.phpt \
        tests/php-5.2.5/Zend/tests/errmsg_038.phpt \
        tests/php-5.2.5/Zend/tests/errmsg_039.phpt \
        tests/php-5.2.5/Zend/tests/errmsg_040.phpt \
        tests/php-5.2.5/Zend/tests/errmsg_041.phpt \
        tests/php-5.2.5/Zend/tests/errmsg_042.phpt \
        tests/php-5.2.5/Zend/tests/errmsg_043.phpt \
        tests/php-5.2.5/Zend/tests/error_reporting01.phpt \
        tests/php-5.2.5/Zend/tests/error_reporting02.phpt \
        tests/php-5.2.5/Zend/tests/error_reporting03.phpt \
        tests/php-5.2.5/Zend/tests/error_reporting04.phpt \
        tests/php-5.2.5/Zend/tests/error_reporting05.phpt \
        tests/php-5.2.5/Zend/tests/error_reporting06.phpt \
        tests/php-5.2.5/Zend/tests/error_reporting07.phpt \
        tests/php-5.2.5/Zend/tests/error_reporting08.phpt \
        tests/php-5.2.5/Zend/tests/error_reporting09.phpt \
        tests/php-5.2.5/Zend/tests/error_reporting10.phpt \
        tests/php-5.2.5/Zend/tests/bug30922.phpt \
        tests/php-5.2.5/Zend/tests/bug31102.phpt \
        tests/php-5.2.5/Zend/tests/bug33318.phpt \
        tests/php-5.2.5/Zend/tests/bug34064.phpt \
        tests/php-5.2.5/Zend/tests/bug36071.phpt \
        tests/php-5.2.5/Zend/tests/bug36268.phpt \
        tests/php-5.2.5/Zend/tests/bug37632.phpt \
        tests/php-5.2.5/Zend/tests/bug40770.phpt \
        tests/php-5.2.5/Zend/tests/bug41117_1.phpt \
        tests/php-5.2.5/Zend/tests/bug41209.phpt \
        tests/php-5.2.5/Zend/tests/bug41351_2.phpt \
        tests/php-5.2.5/Zend/tests/bug41351_3.phpt \
        tests/php-5.2.5/Zend/tests/bug41351.phpt \
        tests/php-5.2.5/Zend/tests/bug41421.phpt \
        tests/php-5.2.5/Zend/tests/bug41633_2.phpt \
        tests/php-5.2.5/Zend/tests/bug41633_3.phpt \
        tests/php-5.2.5/Zend/tests/bug41813.phpt \
        tests/php-5.2.5/Zend/tests/bug42817.phpt \
        tests/php-5.2.5/Zend/tests/double_to_string_64bit.phpt \
        tests/php-5.2.5/Zend/tests/hex_overflow_32bit.phpt \
        tests/php-5.2.5/Zend/tests/int_overflow_64bit.phpt \
        tests/php-5.2.5/Zend/tests/int_underflow_64bit.phpt \
        tests/php-5.2.5/Zend/tests/bug42818.phpt \
        tests/php-5.2.5/Zend/tests/mul_001.phpt \
        tests/php-5.2.5/Zend/tests/not_002.phpt \
        tests/php-5.2.5/Zend/tests/objects_012.phpt \
        tests/php-5.2.5/Zend/tests/objects_013.phpt \
        tests/php-5.2.5/Zend/tests/objects_014.phpt \
        tests/php-5.2.5/Zend/tests/offset_assign.phpt \
        tests/php-5.2.5/Zend/tests/offset_object.phpt \
        tests/php-5.2.5/Zend/tests/017.phpt \

# These succeed, but are testing an expected parser error
# which is not handled gracefully by this makefile
SKIPPED += \
	tests/php-5.2.5/tests/lang/bug21820.phpt  \
        tests/php-5.2.5/tests/lang/033.phpt \
        tests/php-5.2.5/tests/lang/bug24054.phpt \
	tests/php-5.2.5/tests/lang/bug24573.phpt \

# These have some warning message difference, which is not critical (e.g. line#)
# This a positive list to run, not a negative list to filter out
# Many of the SKIPPED tests could be moved here with some work on the filter
FILTERED = \
        tests/php-5.2.5/tests/lang/bug23584.phpt \

FAILED = \
        tests/php-5.2.5/ext/json/tests/bug41403.phpt \
        tests/php-5.2.5/ext/json/tests/pass001.1_64bit.phpt \
        tests/php-5.2.5/ext/json/tests/pass001.1.phpt \
\
        tests/php-5.2.5/ext/date/tests/012.phpt \
        tests/php-5.2.5/ext/date/tests/013.phpt \
        tests/php-5.2.5/ext/date/tests/014.phpt \
        tests/php-5.2.5/ext/date/tests/bug20382-1.phpt \
        tests/php-5.2.5/ext/date/tests/bug20382-2.phpt \
        tests/php-5.2.5/ext/date/tests/bug26090.phpt \
        tests/php-5.2.5/ext/date/tests/bug27719.phpt \
        tests/php-5.2.5/ext/date/tests/bug28024.phpt \
        tests/php-5.2.5/ext/date/tests/bug30096.phpt \
        tests/php-5.2.5/ext/date/tests/bug30532.phpt \
        tests/php-5.2.5/ext/date/tests/bug32086.phpt \
        tests/php-5.2.5/ext/date/tests/bug32270.phpt \
        tests/php-5.2.5/ext/date/tests/bug33414-1.phpt \
        tests/php-5.2.5/ext/date/tests/bug33414-2.phpt \
        tests/php-5.2.5/ext/date/tests/bug33415-1.phpt \
        tests/php-5.2.5/ext/date/tests/bug33415-2.phpt \
        tests/php-5.2.5/ext/date/tests/bug33562.phpt \
        tests/php-5.2.5/ext/date/tests/bug35143.phpt \
        tests/php-5.2.5/ext/date/tests/bug35499.phpt \
        tests/php-5.2.5/ext/date/tests/bug39782.phpt \
        tests/php-5.2.5/ext/date/tests/bug41523.phpt \
        tests/php-5.2.5/ext/date/tests/bug41709.phpt \
        tests/php-5.2.5/ext/date/tests/bug41842.phpt \
        tests/php-5.2.5/ext/date/tests/bug41844.phpt \
        tests/php-5.2.5/ext/date/tests/date_constants.phpt \
        tests/php-5.2.5/ext/date/tests/date_create-1.phpt \
        tests/php-5.2.5/ext/date/tests/date_create-2.phpt \
        tests/php-5.2.5/ext/date/tests/date_modify-1.phpt \
        tests/php-5.2.5/ext/date/tests/date_modify-2.phpt \
        tests/php-5.2.5/ext/date/tests/date_parse_001.phpt \
        tests/php-5.2.5/ext/date/tests/mktime-1.phpt \
        tests/php-5.2.5/ext/date/tests/mktime-2.phpt \
        tests/php-5.2.5/ext/date/tests/oo_001.phpt \
        tests/php-5.2.5/ext/date/tests/oo_002.phpt \
        tests/php-5.2.5/ext/date/tests/strtotime3.phpt \
        tests/php-5.2.5/ext/date/tests/timezones.phpt \
\
        tests/php-5.2.5/sapi/tests/test004.phpt \
	tests/php-5.2.5/tests/lang/039.phpt \
        tests/php-5.2.5/tests/lang/bug21094.phpt \
        tests/php-5.2.5/tests/lang/bug21600.phpt \
        tests/php-5.2.5/tests/lang/bug22592.phpt \
        tests/php-5.2.5/tests/lang/bug23384.phpt \
        tests/php-5.2.5/tests/lang/bug23489.phpt \
        tests/php-5.2.5/tests/lang/bug23624.phpt \
        tests/php-5.2.5/tests/lang/bug24658.phpt \
        tests/php-5.2.5/tests/lang/bug24951.phpt \
	tests/php-5.2.5/tests/lang/bug25145.phpt \
        tests/php-5.2.5/tests/lang/bug25547.phpt \
        tests/php-5.2.5/tests/lang/bug25922.phpt \
        tests/php-5.2.5/tests/lang/bug27439.phpt \
        tests/php-5.2.5/tests/lang/bug28213.phpt \
        tests/php-5.2.5/tests/lang/bug29566.phpt \
        tests/php-5.2.5/tests/lang/bug29893.phpt \
        tests/php-5.2.5/tests/lang/bug30578.phpt \
        tests/php-5.2.5/tests/lang/bug32828.phpt \
        tests/php-5.2.5/tests/lang/bug32924.phpt \
        tests/php-5.2.5/tests/lang/bug35176.phpt \
        tests/php-5.2.5/tests/lang/bug35382.phpt \
        tests/php-5.2.5/tests/lang/bug38579.phpt \
        tests/php-5.2.5/tests/lang/catchable_error_001.phpt \
        tests/php-5.2.5/tests/lang/catchable_error_002.phpt \
        tests/php-5.2.5/tests/lang/error_2_exception_001.phpt \
        tests/php-5.2.5/tests/lang/foreach_with_references_001.phpt \
        tests/php-5.2.5/tests/lang/type_hints_001.phpt \
        tests/php-5.2.5/tests/lang/type_hints_003.phpt \
\
        tests/php-5.2.5/tests/strings/002.phpt \
        tests/php-5.2.5/tests/strings/004.phpt \
        tests/php-5.2.5/tests/strings/bug26703.phpt \
\
        tests/php-5.2.5/tests/func/005a.phpt \
        tests/php-5.2.5/tests/func/007.phpt \
\
        tests/php-5.2.5/tests/run-test/test005.phpt \
        tests/php-5.2.5/tests/run-test/test007.phpt \
        tests/php-5.2.5/tests/run-test/test008a.phpt \
\
        tests/php-5.2.5/tests/classes/abstract_by_interface_001.phpt \
        tests/php-5.2.5/tests/classes/abstract_by_interface_002.phpt \
        tests/php-5.2.5/tests/classes/abstract_class.phpt \
        tests/php-5.2.5/tests/classes/abstract_derived.phpt \
        tests/php-5.2.5/tests/classes/abstract_final.phpt \
        tests/php-5.2.5/tests/classes/abstract_inherit.phpt \
        tests/php-5.2.5/tests/classes/abstract_not_declared.phpt \
        tests/php-5.2.5/tests/classes/abstract.phpt \
        tests/php-5.2.5/tests/classes/abstract_redeclare.phpt \
        tests/php-5.2.5/tests/classes/abstract_static.phpt \
        tests/php-5.2.5/tests/classes/abstract_user_call.phpt \
        tests/php-5.2.5/tests/classes/array_access_001.phpt \
        tests/php-5.2.5/tests/classes/array_access_002.phpt \
        tests/php-5.2.5/tests/classes/array_access_003.phpt \
        tests/php-5.2.5/tests/classes/array_access_004.phpt \
        tests/php-5.2.5/tests/classes/array_access_005.phpt \
        tests/php-5.2.5/tests/classes/array_access_006.phpt \
        tests/php-5.2.5/tests/classes/array_access_008.phpt \
        tests/php-5.2.5/tests/classes/array_access_009.phpt \
        tests/php-5.2.5/tests/classes/array_access_010.phpt \
        tests/php-5.2.5/tests/classes/array_access_011.phpt \
        tests/php-5.2.5/tests/classes/array_access_012.phpt \
        tests/php-5.2.5/tests/classes/autoload_001.phpt \
        tests/php-5.2.5/tests/classes/autoload_002.phpt \
        tests/php-5.2.5/tests/classes/autoload_003.phpt \
        tests/php-5.2.5/tests/classes/autoload_004.phpt \
        tests/php-5.2.5/tests/classes/autoload_005.phpt \
        tests/php-5.2.5/tests/classes/autoload_006.phpt \
        tests/php-5.2.5/tests/classes/bug26737.phpt \
        tests/php-5.2.5/tests/classes/bug27468.phpt \
        tests/php-5.2.5/tests/classes/bug27504.phpt \
        tests/php-5.2.5/tests/classes/bug29446.phpt \
        tests/php-5.2.5/tests/classes/__call_002.phpt \
        tests/php-5.2.5/tests/classes/class_abstract.phpt \
        tests/php-5.2.5/tests/classes/class_final.phpt \
        tests/php-5.2.5/tests/classes/clone_003.phpt \
        tests/php-5.2.5/tests/classes/clone_004.phpt \
        tests/php-5.2.5/tests/classes/clone_005.phpt \
        tests/php-5.2.5/tests/classes/ctor_in_interface_01.phpt \
        tests/php-5.2.5/tests/classes/ctor_in_interface_02.phpt \
        tests/php-5.2.5/tests/classes/ctor_in_interface_03.phpt \
        tests/php-5.2.5/tests/classes/ctor_in_interface_04.phpt \
        tests/php-5.2.5/tests/classes/ctor_visibility.phpt \
        tests/php-5.2.5/tests/classes/destructor_visibility_001.phpt \
        tests/php-5.2.5/tests/classes/destructor_visibility_002.phpt \
        tests/php-5.2.5/tests/classes/factory_and_singleton_002.phpt \
        tests/php-5.2.5/tests/classes/factory_and_singleton_003.phpt \
        tests/php-5.2.5/tests/classes/factory_and_singleton_004.phpt \
        tests/php-5.2.5/tests/classes/factory_and_singleton_005.phpt \
        tests/php-5.2.5/tests/classes/factory_and_singleton_006.phpt \
        tests/php-5.2.5/tests/classes/factory_and_singleton_007.phpt \
        tests/php-5.2.5/tests/classes/factory_and_singleton_008.phpt \
        tests/php-5.2.5/tests/classes/factory_and_singleton_009.phpt \
        tests/php-5.2.5/tests/classes/factory_and_singleton_010.phpt \
        tests/php-5.2.5/tests/classes/final_abstract.phpt \
        tests/php-5.2.5/tests/classes/final_ctor1.phpt \
        tests/php-5.2.5/tests/classes/final_ctor2.phpt \
        tests/php-5.2.5/tests/classes/final_redeclare.phpt \
        tests/php-5.2.5/tests/classes/incdec_property_001.phpt \
        tests/php-5.2.5/tests/classes/incdec_property_002.phpt \
        tests/php-5.2.5/tests/classes/incdec_property_003.phpt \
        tests/php-5.2.5/tests/classes/incdec_property_004.phpt \
        tests/php-5.2.5/tests/classes/inheritance.phpt \
        tests/php-5.2.5/tests/classes/interface_and_extends.phpt \
        tests/php-5.2.5/tests/classes/interface_class.phpt \
        tests/php-5.2.5/tests/classes/interface_doubled.phpt \
        tests/php-5.2.5/tests/classes/interface_implemented.phpt \
        tests/php-5.2.5/tests/classes/interface_instantiate.phpt \
        tests/php-5.2.5/tests/classes/interface_member.phpt \
        tests/php-5.2.5/tests/classes/interface_method_final.phpt \
        tests/php-5.2.5/tests/classes/interface_method.phpt \
        tests/php-5.2.5/tests/classes/interface_method_private.phpt \
        tests/php-5.2.5/tests/classes/interface_must_be_implemented.phpt \
        tests/php-5.2.5/tests/classes/interfaces_002.phpt \
        tests/php-5.2.5/tests/classes/interfaces_003.phpt \
        tests/php-5.2.5/tests/classes/iterators_001.phpt \
        tests/php-5.2.5/tests/classes/iterators_002.phpt \
        tests/php-5.2.5/tests/classes/iterators_003.phpt \
        tests/php-5.2.5/tests/classes/iterators_005.phpt \
        tests/php-5.2.5/tests/classes/iterators_006.phpt \
        tests/php-5.2.5/tests/classes/private_001.phpt \
        tests/php-5.2.5/tests/classes/private_002.phpt \
        tests/php-5.2.5/tests/classes/private_003b.phpt \
        tests/php-5.2.5/tests/classes/private_003.phpt \
        tests/php-5.2.5/tests/classes/private_004b.phpt \
        tests/php-5.2.5/tests/classes/private_004.phpt \
        tests/php-5.2.5/tests/classes/private_005b.phpt \
        tests/php-5.2.5/tests/classes/private_005.phpt \
        tests/php-5.2.5/tests/classes/private_members.phpt \
        tests/php-5.2.5/tests/classes/private_redeclare.phpt \
        tests/php-5.2.5/tests/classes/protected_001b.phpt \
        tests/php-5.2.5/tests/classes/protected_001.phpt \
        tests/php-5.2.5/tests/classes/protected_002.phpt \
        tests/php-5.2.5/tests/classes/serialize_001.phpt \
        tests/php-5.2.5/tests/classes/__set_data_corrupt.phpt \
        tests/php-5.2.5/tests/classes/__set__get_001.phpt \
        tests/php-5.2.5/tests/classes/__set__get_005.phpt \
        tests/php-5.2.5/tests/classes/static_mix_1.phpt \
        tests/php-5.2.5/tests/classes/static_mix_2.phpt \
        tests/php-5.2.5/tests/classes/this.phpt \
        tests/php-5.2.5/tests/classes/tostring_001.phpt \
        tests/php-5.2.5/tests/classes/tostring_003.phpt \
        tests/php-5.2.5/tests/classes/type_hinting_001.phpt \
        tests/php-5.2.5/tests/classes/type_hinting_002.phpt \
        tests/php-5.2.5/tests/classes/type_hinting_003.phpt \
        tests/php-5.2.5/tests/classes/visibility_000a.phpt \
        tests/php-5.2.5/tests/classes/visibility_000b.phpt \
        tests/php-5.2.5/tests/classes/visibility_001a.phpt \
        tests/php-5.2.5/tests/classes/visibility_001b.phpt \
        tests/php-5.2.5/tests/classes/visibility_002a.phpt \
        tests/php-5.2.5/tests/classes/visibility_002b.phpt \
        tests/php-5.2.5/tests/classes/visibility_003b.phpt \
        tests/php-5.2.5/tests/classes/visibility_005.phpt \
\
        tests/php-5.2.5/Zend/tests/008.phpt \
        tests/php-5.2.5/Zend/tests/011.phpt \
        tests/php-5.2.5/Zend/tests/014.phpt \
        tests/php-5.2.5/Zend/tests/018.phpt \
        tests/php-5.2.5/Zend/tests/019.phpt \
        tests/php-5.2.5/Zend/tests/020.phpt \
        tests/php-5.2.5/Zend/tests/array_type_hint_001.phpt \
        tests/php-5.2.5/Zend/tests/bug20240.phpt \
        tests/php-5.2.5/Zend/tests/bug21478.phpt \
        tests/php-5.2.5/Zend/tests/bug24884.phpt \
        tests/php-5.2.5/Zend/tests/bug26166.phpt \
        tests/php-5.2.5/Zend/tests/bug26229.phpt \
        tests/php-5.2.5/Zend/tests/bug27669.phpt \
        tests/php-5.2.5/Zend/tests/bug27731.phpt \
        tests/php-5.2.5/Zend/tests/bug27798.phpt \
        tests/php-5.2.5/Zend/tests/bug28072.phpt \
        tests/php-5.2.5/Zend/tests/bug28377.phpt \
        tests/php-5.2.5/Zend/tests/bug28444.phpt \
        tests/php-5.2.5/Zend/tests/bug29210.phpt \
        tests/php-5.2.5/Zend/tests/bug29505.phpt \
        tests/php-5.2.5/Zend/tests/bug29689.phpt \
        tests/php-5.2.5/Zend/tests/bug29890.phpt \
        tests/php-5.2.5/Zend/tests/bug29896.phpt \
        tests/php-5.2.5/Zend/tests/bug30080.phpt \
        tests/php-5.2.5/Zend/tests/bug30162.phpt \
        tests/php-5.2.5/Zend/tests/bug30725.phpt \
        tests/php-5.2.5/Zend/tests/bug30791.phpt \
        tests/php-5.2.5/Zend/tests/bug30820.phpt \
        tests/php-5.2.5/Zend/tests/bug30828.phpt \
        tests/php-5.2.5/Zend/tests/bug30889.phpt \
        tests/php-5.2.5/Zend/tests/bug30998.phpt \
        tests/php-5.2.5/Zend/tests/bug31098.phpt \
        tests/php-5.2.5/Zend/tests/bug31720.phpt \
        tests/php-5.2.5/Zend/tests/bug32290.phpt \
        tests/php-5.2.5/Zend/tests/bug32296.phpt \
        tests/php-5.2.5/Zend/tests/bug32322.phpt \
        tests/php-5.2.5/Zend/tests/bug33116.phpt \
        tests/php-5.2.5/Zend/tests/bug33277.phpt \
        tests/php-5.2.5/Zend/tests/bug33512.phpt \
        tests/php-5.2.5/Zend/tests/bug33732.phpt \
        tests/php-5.2.5/Zend/tests/bug33771.phpt \
        tests/php-5.2.5/Zend/tests/bug33802.phpt \
        tests/php-5.2.5/Zend/tests/bug33996.phpt \
        tests/php-5.2.5/Zend/tests/bug34062.phpt \
        tests/php-5.2.5/Zend/tests/bug34137.phpt \
        tests/php-5.2.5/Zend/tests/bug34199.phpt \
        tests/php-5.2.5/Zend/tests/bug34518.phpt \
        tests/php-5.2.5/Zend/tests/bug34617.phpt \
        tests/php-5.2.5/Zend/tests/bug34786.phpt \
        tests/php-5.2.5/Zend/tests/bug35017.phpt \
        tests/php-5.2.5/Zend/tests/bug35437.phpt \
        tests/php-5.2.5/Zend/tests/bug35470.phpt \
        tests/php-5.2.5/Zend/tests/bug35634.phpt \
        tests/php-5.2.5/Zend/tests/bug35655.phpt \
        tests/php-5.2.5/Zend/tests/bug36214.phpt \
        tests/php-5.2.5/Zend/tests/bug36303.phpt \
        tests/php-5.2.5/Zend/tests/bug36513.phpt \
        tests/php-5.2.5/Zend/tests/bug36568.phpt \
        tests/php-5.2.5/Zend/tests/bug37138.phpt \
        tests/php-5.2.5/Zend/tests/bug37212.phpt \
        tests/php-5.2.5/Zend/tests/bug37667.phpt \
        tests/php-5.2.5/Zend/tests/bug37811.phpt \
        tests/php-5.2.5/Zend/tests/bug38220.phpt \
        tests/php-5.2.5/Zend/tests/bug38779_1.phpt \
        tests/php-5.2.5/Zend/tests/bug38779.phpt \
        tests/php-5.2.5/Zend/tests/bug38942.phpt \
        tests/php-5.2.5/Zend/tests/bug39003.phpt \
        tests/php-5.2.5/Zend/tests/bug39036.phpt \
        tests/php-5.2.5/Zend/tests/bug39297.phpt \
        tests/php-5.2.5/Zend/tests/bug39304.phpt \
        tests/php-5.2.5/Zend/tests/bug39449.phpt \
        tests/php-5.2.5/Zend/tests/bug39542.phpt \
        tests/php-5.2.5/Zend/tests/bug39944.phpt \
        tests/php-5.2.5/Zend/tests/bug40236.phpt \
        tests/php-5.2.5/Zend/tests/bug40509.phpt \
        tests/php-5.2.5/Zend/tests/bug40621.phpt \
        tests/php-5.2.5/Zend/tests/bug40705.phpt \
        tests/php-5.2.5/Zend/tests/bug40815.phpt \
        tests/php-5.2.5/Zend/tests/bug41026.phpt \
        tests/php-5.2.5/Zend/tests/bug41075.phpt \
        tests/php-5.2.5/Zend/tests/bug41633_4.phpt \
        tests/php-5.2.5/Zend/tests/bug41640.phpt \
        tests/php-5.2.5/Zend/tests/bug41919.phpt \
        tests/php-5.2.5/Zend/tests/bug42211.phpt \
        tests/php-5.2.5/Zend/tests/bug42767.phpt \
        tests/php-5.2.5/Zend/tests/bug_debug_backtrace.phpt \
        tests/php-5.2.5/Zend/tests/cast_to_array.phpt \
        tests/php-5.2.5/Zend/tests/cast_to_double.phpt \
        tests/php-5.2.5/Zend/tests/cast_to_int.phpt \
        tests/php-5.2.5/Zend/tests/cast_to_object.phpt \
        tests/php-5.2.5/Zend/tests/cast_to_string.phpt \
        tests/php-5.2.5/Zend/tests/catch.phpt \
        tests/php-5.2.5/Zend/tests/class_constants_001.phpt \
        tests/php-5.2.5/Zend/tests/class_constants_002.phpt \
        tests/php-5.2.5/Zend/tests/class_constants_003.phpt \
        tests/php-5.2.5/Zend/tests/compare_001_64bit.phpt \
        tests/php-5.2.5/Zend/tests/compare_002_64bit.phpt \
        tests/php-5.2.5/Zend/tests/compare_003_64bit.phpt \
        tests/php-5.2.5/Zend/tests/compare_004_64bit.phpt \
        tests/php-5.2.5/Zend/tests/compare_005_64bit.phpt \
        tests/php-5.2.5/Zend/tests/compare_006_64bit.phpt \
        tests/php-5.2.5/Zend/tests/decrement_001_64bit.phpt \
        tests/php-5.2.5/Zend/tests/div_002.phpt \
        tests/php-5.2.5/Zend/tests/exception_handler_002.phpt \
        tests/php-5.2.5/Zend/tests/exception_handler_003.phpt \
        tests/php-5.2.5/Zend/tests/exception_handler_004.phpt \
        tests/php-5.2.5/Zend/tests/foreach_undefined.phpt \
        tests/php-5.2.5/Zend/tests/get_defined_vars.phpt \
        tests/php-5.2.5/Zend/tests/globals_001.phpt \
        tests/php-5.2.5/Zend/tests/globals_002.phpt \
        tests/php-5.2.5/Zend/tests/globals_003.phpt \
        tests/php-5.2.5/Zend/tests/globals_004.phpt \
        tests/php-5.2.5/Zend/tests/halt01.phpt \
        tests/php-5.2.5/Zend/tests/halt02.phpt \
        tests/php-5.2.5/Zend/tests/halt03.phpt \
        tests/php-5.2.5/Zend/tests/halt_compiler1.phpt \
        tests/php-5.2.5/Zend/tests/halt_compiler2.phpt \
        tests/php-5.2.5/Zend/tests/halt_compiler4.phpt \
        tests/php-5.2.5/Zend/tests/instanceof.phpt \
        tests/php-5.2.5/Zend/tests/is_a.phpt \
        tests/php-5.2.5/Zend/tests/magic_by_ref_001.phpt \
        tests/php-5.2.5/Zend/tests/magic_by_ref_002.phpt \
        tests/php-5.2.5/Zend/tests/magic_by_ref_003.phpt \
        tests/php-5.2.5/Zend/tests/magic_by_ref_004.phpt \
        tests/php-5.2.5/Zend/tests/magic_by_ref_005.phpt \
        tests/php-5.2.5/Zend/tests/magic_by_ref_006.phpt \
        tests/php-5.2.5/Zend/tests/magic_by_ref_007.phpt \
        tests/php-5.2.5/Zend/tests/mod_001.phpt \
        tests/php-5.2.5/Zend/tests/object_handlers.phpt \
        tests/php-5.2.5/Zend/tests/objects_010.phpt \
        tests/php-5.2.5/Zend/tests/objects_011.phpt \
        tests/php-5.2.5/Zend/tests/oct_overflow_32bit.phpt \
        tests/php-5.2.5/Zend/tests/offset_array.phpt \
        tests/php-5.2.5/Zend/tests/settype_array.phpt \
        tests/php-5.2.5/Zend/tests/settype_double.phpt \
        tests/php-5.2.5/Zend/tests/settype_int.phpt \
        tests/php-5.2.5/Zend/tests/settype_object.phpt \
        tests/php-5.2.5/Zend/tests/settype_resource.phpt \
        tests/php-5.2.5/Zend/tests/settype_string.phpt \
        tests/php-5.2.5/Zend/tests/strict_001.phpt \
        tests/php-5.2.5/Zend/tests/sub_001.phpt \
        tests/php-5.2.5/Zend/tests/unset_cv05.phpt \
        tests/php-5.2.5/Zend/tests/unset_cv06.phpt \
        tests/php-5.2.5/Zend/tests/unset_cv07.phpt \
        tests/php-5.2.5/Zend/tests/unset_cv09.phpt \
        tests/php-5.2.5/Zend/tests/unset_cv10.phpt \

FAILED += tests/php-5.2.5/tests/lang/bug22231.phpt # Bug 155049 
FAILED += tests/php-5.2.5/tests/lang/038.phpt # bug 155077



######## Unsupported Features

# private overrides private, noted in doc/inconsistencies 
FAILED += tests/php-5.2.5/tests/lang/036.phpt 
# eval not supported 
FAILED += \
        tests/php-5.2.5/tests/lang/013.phpt \
        tests/php-5.2.5/tests/lang/014.phpt \
        tests/php-5.2.5/tests/lang/018.phpt \
        tests/php-5.2.5/tests/lang/019.phpt \
        tests/php-5.2.5/tests/lang/bug21961.phpt \

FAILED += tests/php-5.2.5/tests/lang/bug24403.phpt # no preg_replace
FAILED += tests/php-5.2.5/tests/lang/bug24396.phpt # dynamic global 
# get_declared_classes is much different in HPHP than PHP 
FAILED += tests/php-5.2.5/tests/lang/bug19566.phpt 
# lambda/create_function issues 
FAILED += \
        tests/php-5.2.5/tests/lang/bug17115.phpt \
        tests/php-5.2.5/tests/lang/bug22690.phpt \



RUNS = $(patsubst %.phpt, %.run, \
         $(filter-out $(SKIPPED) $(FILTERED) $(FAILED), $(PHPT_FILES)))

RUNS_FILTERED = $(patsubst %.phpt, %.run_filtered, $(FILTERED))

all: $(RUNS_FILTERED) $(RUNS) 

%.run: %.phpt
	@echo $<
	@php $< > php.out
	@../src/hphp/hphp --input-dir=$(CURDIR) $< > hphp.out
	@diff php.out hphp.out

FILTER = egrep -v "Undefined variable"
%.run_filtered: %.phpt
	@echo $<
	@php $< | $(PHP) > php.out
	@../src/hphp/hphp --input-dir=$(CURDIR) $< | $(PHP) > hphp.out
	@diff php.out hphp.out

%.debug: %.phpt
	../src/hphp/hphp $< --log=3 --keep-tempdir=1

clean:
	rm -f php.out hphp.out *~

clobber: clean
