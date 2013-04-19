/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
   | Copyright (c) 1997-2010 The PHP Group                                |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/

#include "runtime/ext_hhvm/ext_hhvm.h"
#include "runtime/base/builtin_functions.h"
#include "runtime/base/array/array_init.h"
#include "runtime/ext/ext.h"
#include "runtime/vm/class.h"
#include "runtime/vm/runtime.h"
#include <exception>

namespace HPHP {

TypedValue* fh_xmlwriter_open_memory(TypedValue* _rv) asm("_ZN4HPHP23f_xmlwriter_open_memoryEv");

TypedValue* fg_xmlwriter_open_memory(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 0) {
    fh_xmlwriter_open_memory(rv);
    if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  } else {
    throw_toomany_arguments_nr("xmlwriter_open_memory", 0, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool th_9XMLWriter_openMemory(ObjectData* this_) asm("_ZN4HPHP11c_XMLWriter12t_openmemoryEv");

TypedValue* tg_9XMLWriter_openMemory(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 0) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (th_9XMLWriter_openMemory((this_))) ? 1LL : 0LL;
    } else {
      throw_toomany_arguments_nr("XMLWriter::openMemory", 0, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("XMLWriter::openMemory");
  }
  frame_free_locals_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_xmlwriter_open_uri(Value* _rv, Value* uri) asm("_ZN4HPHP20f_xmlwriter_open_uriERKNS_6StringE");

void fg1_xmlwriter_open_uri(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_xmlwriter_open_uri(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  rv->m_type = KindOfObject;
  fh_xmlwriter_open_uri(&(rv->m_data), &args[-0].m_data);
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
}

TypedValue* fg_xmlwriter_open_uri(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if (IS_STRING_TYPE((args - 0)->m_type)) {
      rv->m_type = KindOfObject;
      fh_xmlwriter_open_uri(&(rv->m_data), &args[-0].m_data);
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      fg1_xmlwriter_open_uri(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("xmlwriter_open_uri", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool th_9XMLWriter_openURI(ObjectData* this_, Value* uri) asm("_ZN4HPHP11c_XMLWriter9t_openuriERKNS_6StringE");

void tg1_9XMLWriter_openURI(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) __attribute__((noinline,cold));
void tg1_9XMLWriter_openURI(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (th_9XMLWriter_openURI((this_), &args[-0].m_data)) ? 1LL : 0LL;
}

TypedValue* tg_9XMLWriter_openURI(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 1) {
      if (IS_STRING_TYPE((args - 0)->m_type)) {
        rv->m_type = KindOfBoolean;
        rv->m_data.num = (th_9XMLWriter_openURI((this_), &args[-0].m_data)) ? 1LL : 0LL;
      } else {
        tg1_9XMLWriter_openURI(rv, ar, count, this_);
      }
    } else {
      throw_wrong_arguments_nr("XMLWriter::openURI", count, 1, 1, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("XMLWriter::openURI");
  }
  frame_free_locals_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_xmlwriter_set_indent_string(Value* xmlwriter, Value* indentstring) asm("_ZN4HPHP29f_xmlwriter_set_indent_stringERKNS_6ObjectERKNS_6StringE");

void fg1_xmlwriter_set_indent_string(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_xmlwriter_set_indent_string(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_xmlwriter_set_indent_string(&args[-0].m_data, &args[-1].m_data)) ? 1LL : 0LL;
}

TypedValue* fg_xmlwriter_set_indent_string(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if (IS_STRING_TYPE((args - 1)->m_type) &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_xmlwriter_set_indent_string(&args[-0].m_data, &args[-1].m_data)) ? 1LL : 0LL;
    } else {
      fg1_xmlwriter_set_indent_string(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("xmlwriter_set_indent_string", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool th_9XMLWriter_setIndentString(ObjectData* this_, Value* indentstring) asm("_ZN4HPHP11c_XMLWriter17t_setindentstringERKNS_6StringE");

void tg1_9XMLWriter_setIndentString(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) __attribute__((noinline,cold));
void tg1_9XMLWriter_setIndentString(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (th_9XMLWriter_setIndentString((this_), &args[-0].m_data)) ? 1LL : 0LL;
}

TypedValue* tg_9XMLWriter_setIndentString(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 1) {
      if (IS_STRING_TYPE((args - 0)->m_type)) {
        rv->m_type = KindOfBoolean;
        rv->m_data.num = (th_9XMLWriter_setIndentString((this_), &args[-0].m_data)) ? 1LL : 0LL;
      } else {
        tg1_9XMLWriter_setIndentString(rv, ar, count, this_);
      }
    } else {
      throw_wrong_arguments_nr("XMLWriter::setIndentString", count, 1, 1, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("XMLWriter::setIndentString");
  }
  frame_free_locals_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_xmlwriter_set_indent(Value* xmlwriter, bool indent) asm("_ZN4HPHP22f_xmlwriter_set_indentERKNS_6ObjectEb");

void fg1_xmlwriter_set_indent(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_xmlwriter_set_indent(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-1)->m_type != KindOfBoolean) {
    tvCastToBooleanInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_xmlwriter_set_indent(&args[-0].m_data, (bool)(args[-1].m_data.num))) ? 1LL : 0LL;
}

TypedValue* fg_xmlwriter_set_indent(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if ((args - 1)->m_type == KindOfBoolean &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_xmlwriter_set_indent(&args[-0].m_data, (bool)(args[-1].m_data.num))) ? 1LL : 0LL;
    } else {
      fg1_xmlwriter_set_indent(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("xmlwriter_set_indent", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool th_9XMLWriter_setIndent(ObjectData* this_, bool indent) asm("_ZN4HPHP11c_XMLWriter11t_setindentEb");

void tg1_9XMLWriter_setIndent(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) __attribute__((noinline,cold));
void tg1_9XMLWriter_setIndent(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToBooleanInPlace(args-0);
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (th_9XMLWriter_setIndent((this_), (bool)(args[-0].m_data.num))) ? 1LL : 0LL;
}

TypedValue* tg_9XMLWriter_setIndent(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 1) {
      if ((args - 0)->m_type == KindOfBoolean) {
        rv->m_type = KindOfBoolean;
        rv->m_data.num = (th_9XMLWriter_setIndent((this_), (bool)(args[-0].m_data.num))) ? 1LL : 0LL;
      } else {
        tg1_9XMLWriter_setIndent(rv, ar, count, this_);
      }
    } else {
      throw_wrong_arguments_nr("XMLWriter::setIndent", count, 1, 1, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("XMLWriter::setIndent");
  }
  frame_free_locals_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_xmlwriter_start_document(Value* xmlwriter, Value* version, Value* encoding, Value* standalone) asm("_ZN4HPHP26f_xmlwriter_start_documentERKNS_6ObjectERKNS_6StringES5_S5_");

void fg1_xmlwriter_start_document(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_xmlwriter_start_document(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 4
    if (!IS_STRING_TYPE((args-3)->m_type)) {
      tvCastToStringInPlace(args-3);
    }
  case 3:
    if (!IS_STRING_TYPE((args-2)->m_type)) {
      tvCastToStringInPlace(args-2);
    }
  case 2:
    if (!IS_STRING_TYPE((args-1)->m_type)) {
      tvCastToStringInPlace(args-1);
    }
  case 1:
    break;
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  String defVal1 = "1.0";
  rv->m_data.num = (fh_xmlwriter_start_document(&args[-0].m_data, (count > 1) ? &args[-1].m_data : (Value*)(&defVal1), (count > 2) ? &args[-2].m_data : (Value*)(&null_string), (count > 3) ? &args[-3].m_data : (Value*)(&null_string))) ? 1LL : 0LL;
}

TypedValue* fg_xmlwriter_start_document(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 1 && count <= 4) {
    if ((count <= 3 || IS_STRING_TYPE((args - 3)->m_type)) &&
        (count <= 2 || IS_STRING_TYPE((args - 2)->m_type)) &&
        (count <= 1 || IS_STRING_TYPE((args - 1)->m_type)) &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      String defVal1 = "1.0";
      rv->m_data.num = (fh_xmlwriter_start_document(&args[-0].m_data, (count > 1) ? &args[-1].m_data : (Value*)(&defVal1), (count > 2) ? &args[-2].m_data : (Value*)(&null_string), (count > 3) ? &args[-3].m_data : (Value*)(&null_string))) ? 1LL : 0LL;
    } else {
      fg1_xmlwriter_start_document(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("xmlwriter_start_document", count, 1, 4, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 4);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool th_9XMLWriter_startDocument(ObjectData* this_, Value* version, Value* encoding, Value* standalone) asm("_ZN4HPHP11c_XMLWriter15t_startdocumentERKNS_6StringES3_S3_");

void tg1_9XMLWriter_startDocument(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) __attribute__((noinline,cold));
void tg1_9XMLWriter_startDocument(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 3
    if (!IS_STRING_TYPE((args-2)->m_type)) {
      tvCastToStringInPlace(args-2);
    }
  case 2:
    if (!IS_STRING_TYPE((args-1)->m_type)) {
      tvCastToStringInPlace(args-1);
    }
  case 1:
    if (!IS_STRING_TYPE((args-0)->m_type)) {
      tvCastToStringInPlace(args-0);
    }
  case 0:
    break;
  }
  rv->m_type = KindOfBoolean;
  String defVal0 = "1.0";
  rv->m_data.num = (th_9XMLWriter_startDocument((this_), (count > 0) ? &args[-0].m_data : (Value*)(&defVal0), (count > 1) ? &args[-1].m_data : (Value*)(&null_string), (count > 2) ? &args[-2].m_data : (Value*)(&null_string))) ? 1LL : 0LL;
}

TypedValue* tg_9XMLWriter_startDocument(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count <= 3) {
      if ((count <= 2 || IS_STRING_TYPE((args - 2)->m_type)) &&
          (count <= 1 || IS_STRING_TYPE((args - 1)->m_type)) &&
          (count <= 0 || IS_STRING_TYPE((args - 0)->m_type))) {
        rv->m_type = KindOfBoolean;
        String defVal0 = "1.0";
        rv->m_data.num = (th_9XMLWriter_startDocument((this_), (count > 0) ? &args[-0].m_data : (Value*)(&defVal0), (count > 1) ? &args[-1].m_data : (Value*)(&null_string), (count > 2) ? &args[-2].m_data : (Value*)(&null_string))) ? 1LL : 0LL;
      } else {
        tg1_9XMLWriter_startDocument(rv, ar, count, this_);
      }
    } else {
      throw_toomany_arguments_nr("XMLWriter::startDocument", 3, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("XMLWriter::startDocument");
  }
  frame_free_locals_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_xmlwriter_start_element(Value* xmlwriter, Value* name) asm("_ZN4HPHP25f_xmlwriter_start_elementERKNS_6ObjectERKNS_6StringE");

void fg1_xmlwriter_start_element(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_xmlwriter_start_element(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_xmlwriter_start_element(&args[-0].m_data, &args[-1].m_data)) ? 1LL : 0LL;
}

TypedValue* fg_xmlwriter_start_element(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if (IS_STRING_TYPE((args - 1)->m_type) &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_xmlwriter_start_element(&args[-0].m_data, &args[-1].m_data)) ? 1LL : 0LL;
    } else {
      fg1_xmlwriter_start_element(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("xmlwriter_start_element", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool th_9XMLWriter_startElement(ObjectData* this_, Value* name) asm("_ZN4HPHP11c_XMLWriter14t_startelementERKNS_6StringE");

void tg1_9XMLWriter_startElement(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) __attribute__((noinline,cold));
void tg1_9XMLWriter_startElement(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (th_9XMLWriter_startElement((this_), &args[-0].m_data)) ? 1LL : 0LL;
}

TypedValue* tg_9XMLWriter_startElement(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 1) {
      if (IS_STRING_TYPE((args - 0)->m_type)) {
        rv->m_type = KindOfBoolean;
        rv->m_data.num = (th_9XMLWriter_startElement((this_), &args[-0].m_data)) ? 1LL : 0LL;
      } else {
        tg1_9XMLWriter_startElement(rv, ar, count, this_);
      }
    } else {
      throw_wrong_arguments_nr("XMLWriter::startElement", count, 1, 1, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("XMLWriter::startElement");
  }
  frame_free_locals_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_xmlwriter_start_element_ns(Value* xmlwriter, Value* prefix, Value* name, Value* uri) asm("_ZN4HPHP28f_xmlwriter_start_element_nsERKNS_6ObjectERKNS_6StringES5_S5_");

void fg1_xmlwriter_start_element_ns(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_xmlwriter_start_element_ns(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (!IS_STRING_TYPE((args-3)->m_type)) {
    tvCastToStringInPlace(args-3);
  }
  if (!IS_STRING_TYPE((args-2)->m_type)) {
    tvCastToStringInPlace(args-2);
  }
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_xmlwriter_start_element_ns(&args[-0].m_data, &args[-1].m_data, &args[-2].m_data, &args[-3].m_data)) ? 1LL : 0LL;
}

TypedValue* fg_xmlwriter_start_element_ns(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 4) {
    if (IS_STRING_TYPE((args - 3)->m_type) &&
        IS_STRING_TYPE((args - 2)->m_type) &&
        IS_STRING_TYPE((args - 1)->m_type) &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_xmlwriter_start_element_ns(&args[-0].m_data, &args[-1].m_data, &args[-2].m_data, &args[-3].m_data)) ? 1LL : 0LL;
    } else {
      fg1_xmlwriter_start_element_ns(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("xmlwriter_start_element_ns", count, 4, 4, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 4);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool th_9XMLWriter_startElementNS(ObjectData* this_, Value* prefix, Value* name, Value* uri) asm("_ZN4HPHP11c_XMLWriter16t_startelementnsERKNS_6StringES3_S3_");

void tg1_9XMLWriter_startElementNS(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) __attribute__((noinline,cold));
void tg1_9XMLWriter_startElementNS(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (!IS_STRING_TYPE((args-2)->m_type)) {
    tvCastToStringInPlace(args-2);
  }
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (th_9XMLWriter_startElementNS((this_), &args[-0].m_data, &args[-1].m_data, &args[-2].m_data)) ? 1LL : 0LL;
}

TypedValue* tg_9XMLWriter_startElementNS(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 3) {
      if (IS_STRING_TYPE((args - 2)->m_type) &&
          IS_STRING_TYPE((args - 1)->m_type) &&
          IS_STRING_TYPE((args - 0)->m_type)) {
        rv->m_type = KindOfBoolean;
        rv->m_data.num = (th_9XMLWriter_startElementNS((this_), &args[-0].m_data, &args[-1].m_data, &args[-2].m_data)) ? 1LL : 0LL;
      } else {
        tg1_9XMLWriter_startElementNS(rv, ar, count, this_);
      }
    } else {
      throw_wrong_arguments_nr("XMLWriter::startElementNS", count, 3, 3, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("XMLWriter::startElementNS");
  }
  frame_free_locals_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_xmlwriter_write_element_ns(Value* xmlwriter, Value* prefix, Value* name, Value* uri, Value* content) asm("_ZN4HPHP28f_xmlwriter_write_element_nsERKNS_6ObjectERKNS_6StringES5_S5_S5_");

void fg1_xmlwriter_write_element_ns(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_xmlwriter_write_element_ns(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 5
    if (!IS_STRING_TYPE((args-4)->m_type)) {
      tvCastToStringInPlace(args-4);
    }
  case 4:
    break;
  }
  if (!IS_STRING_TYPE((args-3)->m_type)) {
    tvCastToStringInPlace(args-3);
  }
  if (!IS_STRING_TYPE((args-2)->m_type)) {
    tvCastToStringInPlace(args-2);
  }
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_xmlwriter_write_element_ns(&args[-0].m_data, &args[-1].m_data, &args[-2].m_data, &args[-3].m_data, (count > 4) ? &args[-4].m_data : (Value*)(&null_string))) ? 1LL : 0LL;
}

TypedValue* fg_xmlwriter_write_element_ns(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 4 && count <= 5) {
    if ((count <= 4 || IS_STRING_TYPE((args - 4)->m_type)) &&
        IS_STRING_TYPE((args - 3)->m_type) &&
        IS_STRING_TYPE((args - 2)->m_type) &&
        IS_STRING_TYPE((args - 1)->m_type) &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_xmlwriter_write_element_ns(&args[-0].m_data, &args[-1].m_data, &args[-2].m_data, &args[-3].m_data, (count > 4) ? &args[-4].m_data : (Value*)(&null_string))) ? 1LL : 0LL;
    } else {
      fg1_xmlwriter_write_element_ns(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("xmlwriter_write_element_ns", count, 4, 5, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 5);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool th_9XMLWriter_writeElementNS(ObjectData* this_, Value* prefix, Value* name, Value* uri, Value* content) asm("_ZN4HPHP11c_XMLWriter16t_writeelementnsERKNS_6StringES3_S3_S3_");

void tg1_9XMLWriter_writeElementNS(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) __attribute__((noinline,cold));
void tg1_9XMLWriter_writeElementNS(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 4
    if (!IS_STRING_TYPE((args-3)->m_type)) {
      tvCastToStringInPlace(args-3);
    }
  case 3:
    break;
  }
  if (!IS_STRING_TYPE((args-2)->m_type)) {
    tvCastToStringInPlace(args-2);
  }
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (th_9XMLWriter_writeElementNS((this_), &args[-0].m_data, &args[-1].m_data, &args[-2].m_data, (count > 3) ? &args[-3].m_data : (Value*)(&null_string))) ? 1LL : 0LL;
}

TypedValue* tg_9XMLWriter_writeElementNS(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count >= 3 && count <= 4) {
      if ((count <= 3 || IS_STRING_TYPE((args - 3)->m_type)) &&
          IS_STRING_TYPE((args - 2)->m_type) &&
          IS_STRING_TYPE((args - 1)->m_type) &&
          IS_STRING_TYPE((args - 0)->m_type)) {
        rv->m_type = KindOfBoolean;
        rv->m_data.num = (th_9XMLWriter_writeElementNS((this_), &args[-0].m_data, &args[-1].m_data, &args[-2].m_data, (count > 3) ? &args[-3].m_data : (Value*)(&null_string))) ? 1LL : 0LL;
      } else {
        tg1_9XMLWriter_writeElementNS(rv, ar, count, this_);
      }
    } else {
      throw_wrong_arguments_nr("XMLWriter::writeElementNS", count, 3, 4, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("XMLWriter::writeElementNS");
  }
  frame_free_locals_inl(ar, 4);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_xmlwriter_write_element(Value* xmlwriter, Value* name, Value* content) asm("_ZN4HPHP25f_xmlwriter_write_elementERKNS_6ObjectERKNS_6StringES5_");

void fg1_xmlwriter_write_element(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_xmlwriter_write_element(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 3
    if (!IS_STRING_TYPE((args-2)->m_type)) {
      tvCastToStringInPlace(args-2);
    }
  case 2:
    break;
  }
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_xmlwriter_write_element(&args[-0].m_data, &args[-1].m_data, (count > 2) ? &args[-2].m_data : (Value*)(&null_string))) ? 1LL : 0LL;
}

TypedValue* fg_xmlwriter_write_element(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 2 && count <= 3) {
    if ((count <= 2 || IS_STRING_TYPE((args - 2)->m_type)) &&
        IS_STRING_TYPE((args - 1)->m_type) &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_xmlwriter_write_element(&args[-0].m_data, &args[-1].m_data, (count > 2) ? &args[-2].m_data : (Value*)(&null_string))) ? 1LL : 0LL;
    } else {
      fg1_xmlwriter_write_element(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("xmlwriter_write_element", count, 2, 3, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool th_9XMLWriter_writeElement(ObjectData* this_, Value* name, Value* content) asm("_ZN4HPHP11c_XMLWriter14t_writeelementERKNS_6StringES3_");

void tg1_9XMLWriter_writeElement(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) __attribute__((noinline,cold));
void tg1_9XMLWriter_writeElement(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 2
    if (!IS_STRING_TYPE((args-1)->m_type)) {
      tvCastToStringInPlace(args-1);
    }
  case 1:
    break;
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (th_9XMLWriter_writeElement((this_), &args[-0].m_data, (count > 1) ? &args[-1].m_data : (Value*)(&null_string))) ? 1LL : 0LL;
}

TypedValue* tg_9XMLWriter_writeElement(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count >= 1 && count <= 2) {
      if ((count <= 1 || IS_STRING_TYPE((args - 1)->m_type)) &&
          IS_STRING_TYPE((args - 0)->m_type)) {
        rv->m_type = KindOfBoolean;
        rv->m_data.num = (th_9XMLWriter_writeElement((this_), &args[-0].m_data, (count > 1) ? &args[-1].m_data : (Value*)(&null_string))) ? 1LL : 0LL;
      } else {
        tg1_9XMLWriter_writeElement(rv, ar, count, this_);
      }
    } else {
      throw_wrong_arguments_nr("XMLWriter::writeElement", count, 1, 2, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("XMLWriter::writeElement");
  }
  frame_free_locals_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_xmlwriter_end_element(Value* xmlwriter) asm("_ZN4HPHP23f_xmlwriter_end_elementERKNS_6ObjectE");

void fg1_xmlwriter_end_element(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_xmlwriter_end_element(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_xmlwriter_end_element(&args[-0].m_data)) ? 1LL : 0LL;
}

TypedValue* fg_xmlwriter_end_element(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_xmlwriter_end_element(&args[-0].m_data)) ? 1LL : 0LL;
    } else {
      fg1_xmlwriter_end_element(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("xmlwriter_end_element", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool th_9XMLWriter_endElement(ObjectData* this_) asm("_ZN4HPHP11c_XMLWriter12t_endelementEv");

TypedValue* tg_9XMLWriter_endElement(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 0) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (th_9XMLWriter_endElement((this_))) ? 1LL : 0LL;
    } else {
      throw_toomany_arguments_nr("XMLWriter::endElement", 0, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("XMLWriter::endElement");
  }
  frame_free_locals_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_xmlwriter_full_end_element(Value* xmlwriter) asm("_ZN4HPHP28f_xmlwriter_full_end_elementERKNS_6ObjectE");

void fg1_xmlwriter_full_end_element(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_xmlwriter_full_end_element(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_xmlwriter_full_end_element(&args[-0].m_data)) ? 1LL : 0LL;
}

TypedValue* fg_xmlwriter_full_end_element(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_xmlwriter_full_end_element(&args[-0].m_data)) ? 1LL : 0LL;
    } else {
      fg1_xmlwriter_full_end_element(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("xmlwriter_full_end_element", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool th_9XMLWriter_fullEndElement(ObjectData* this_) asm("_ZN4HPHP11c_XMLWriter16t_fullendelementEv");

TypedValue* tg_9XMLWriter_fullEndElement(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 0) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (th_9XMLWriter_fullEndElement((this_))) ? 1LL : 0LL;
    } else {
      throw_toomany_arguments_nr("XMLWriter::fullEndElement", 0, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("XMLWriter::fullEndElement");
  }
  frame_free_locals_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_xmlwriter_start_attribute_ns(Value* xmlwriter, Value* prefix, Value* name, Value* uri) asm("_ZN4HPHP30f_xmlwriter_start_attribute_nsERKNS_6ObjectERKNS_6StringES5_S5_");

void fg1_xmlwriter_start_attribute_ns(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_xmlwriter_start_attribute_ns(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (!IS_STRING_TYPE((args-3)->m_type)) {
    tvCastToStringInPlace(args-3);
  }
  if (!IS_STRING_TYPE((args-2)->m_type)) {
    tvCastToStringInPlace(args-2);
  }
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_xmlwriter_start_attribute_ns(&args[-0].m_data, &args[-1].m_data, &args[-2].m_data, &args[-3].m_data)) ? 1LL : 0LL;
}

TypedValue* fg_xmlwriter_start_attribute_ns(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 4) {
    if (IS_STRING_TYPE((args - 3)->m_type) &&
        IS_STRING_TYPE((args - 2)->m_type) &&
        IS_STRING_TYPE((args - 1)->m_type) &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_xmlwriter_start_attribute_ns(&args[-0].m_data, &args[-1].m_data, &args[-2].m_data, &args[-3].m_data)) ? 1LL : 0LL;
    } else {
      fg1_xmlwriter_start_attribute_ns(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("xmlwriter_start_attribute_ns", count, 4, 4, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 4);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool th_9XMLWriter_startAttributens(ObjectData* this_, Value* prefix, Value* name, Value* uri) asm("_ZN4HPHP11c_XMLWriter18t_startattributensERKNS_6StringES3_S3_");

void tg1_9XMLWriter_startAttributens(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) __attribute__((noinline,cold));
void tg1_9XMLWriter_startAttributens(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (!IS_STRING_TYPE((args-2)->m_type)) {
    tvCastToStringInPlace(args-2);
  }
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (th_9XMLWriter_startAttributens((this_), &args[-0].m_data, &args[-1].m_data, &args[-2].m_data)) ? 1LL : 0LL;
}

TypedValue* tg_9XMLWriter_startAttributens(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 3) {
      if (IS_STRING_TYPE((args - 2)->m_type) &&
          IS_STRING_TYPE((args - 1)->m_type) &&
          IS_STRING_TYPE((args - 0)->m_type)) {
        rv->m_type = KindOfBoolean;
        rv->m_data.num = (th_9XMLWriter_startAttributens((this_), &args[-0].m_data, &args[-1].m_data, &args[-2].m_data)) ? 1LL : 0LL;
      } else {
        tg1_9XMLWriter_startAttributens(rv, ar, count, this_);
      }
    } else {
      throw_wrong_arguments_nr("XMLWriter::startAttributens", count, 3, 3, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("XMLWriter::startAttributens");
  }
  frame_free_locals_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_xmlwriter_start_attribute(Value* xmlwriter, Value* name) asm("_ZN4HPHP27f_xmlwriter_start_attributeERKNS_6ObjectERKNS_6StringE");

void fg1_xmlwriter_start_attribute(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_xmlwriter_start_attribute(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_xmlwriter_start_attribute(&args[-0].m_data, &args[-1].m_data)) ? 1LL : 0LL;
}

TypedValue* fg_xmlwriter_start_attribute(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if (IS_STRING_TYPE((args - 1)->m_type) &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_xmlwriter_start_attribute(&args[-0].m_data, &args[-1].m_data)) ? 1LL : 0LL;
    } else {
      fg1_xmlwriter_start_attribute(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("xmlwriter_start_attribute", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool th_9XMLWriter_startAttribute(ObjectData* this_, Value* name) asm("_ZN4HPHP11c_XMLWriter16t_startattributeERKNS_6StringE");

void tg1_9XMLWriter_startAttribute(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) __attribute__((noinline,cold));
void tg1_9XMLWriter_startAttribute(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (th_9XMLWriter_startAttribute((this_), &args[-0].m_data)) ? 1LL : 0LL;
}

TypedValue* tg_9XMLWriter_startAttribute(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 1) {
      if (IS_STRING_TYPE((args - 0)->m_type)) {
        rv->m_type = KindOfBoolean;
        rv->m_data.num = (th_9XMLWriter_startAttribute((this_), &args[-0].m_data)) ? 1LL : 0LL;
      } else {
        tg1_9XMLWriter_startAttribute(rv, ar, count, this_);
      }
    } else {
      throw_wrong_arguments_nr("XMLWriter::startAttribute", count, 1, 1, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("XMLWriter::startAttribute");
  }
  frame_free_locals_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_xmlwriter_write_attribute_ns(Value* xmlwriter, Value* prefix, Value* name, Value* uri, Value* content) asm("_ZN4HPHP30f_xmlwriter_write_attribute_nsERKNS_6ObjectERKNS_6StringES5_S5_S5_");

void fg1_xmlwriter_write_attribute_ns(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_xmlwriter_write_attribute_ns(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (!IS_STRING_TYPE((args-4)->m_type)) {
    tvCastToStringInPlace(args-4);
  }
  if (!IS_STRING_TYPE((args-3)->m_type)) {
    tvCastToStringInPlace(args-3);
  }
  if (!IS_STRING_TYPE((args-2)->m_type)) {
    tvCastToStringInPlace(args-2);
  }
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_xmlwriter_write_attribute_ns(&args[-0].m_data, &args[-1].m_data, &args[-2].m_data, &args[-3].m_data, &args[-4].m_data)) ? 1LL : 0LL;
}

TypedValue* fg_xmlwriter_write_attribute_ns(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 5) {
    if (IS_STRING_TYPE((args - 4)->m_type) &&
        IS_STRING_TYPE((args - 3)->m_type) &&
        IS_STRING_TYPE((args - 2)->m_type) &&
        IS_STRING_TYPE((args - 1)->m_type) &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_xmlwriter_write_attribute_ns(&args[-0].m_data, &args[-1].m_data, &args[-2].m_data, &args[-3].m_data, &args[-4].m_data)) ? 1LL : 0LL;
    } else {
      fg1_xmlwriter_write_attribute_ns(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("xmlwriter_write_attribute_ns", count, 5, 5, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 5);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool th_9XMLWriter_writeAttributeNS(ObjectData* this_, Value* prefix, Value* name, Value* uri, Value* content) asm("_ZN4HPHP11c_XMLWriter18t_writeattributensERKNS_6StringES3_S3_S3_");

void tg1_9XMLWriter_writeAttributeNS(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) __attribute__((noinline,cold));
void tg1_9XMLWriter_writeAttributeNS(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (!IS_STRING_TYPE((args-3)->m_type)) {
    tvCastToStringInPlace(args-3);
  }
  if (!IS_STRING_TYPE((args-2)->m_type)) {
    tvCastToStringInPlace(args-2);
  }
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (th_9XMLWriter_writeAttributeNS((this_), &args[-0].m_data, &args[-1].m_data, &args[-2].m_data, &args[-3].m_data)) ? 1LL : 0LL;
}

TypedValue* tg_9XMLWriter_writeAttributeNS(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 4) {
      if (IS_STRING_TYPE((args - 3)->m_type) &&
          IS_STRING_TYPE((args - 2)->m_type) &&
          IS_STRING_TYPE((args - 1)->m_type) &&
          IS_STRING_TYPE((args - 0)->m_type)) {
        rv->m_type = KindOfBoolean;
        rv->m_data.num = (th_9XMLWriter_writeAttributeNS((this_), &args[-0].m_data, &args[-1].m_data, &args[-2].m_data, &args[-3].m_data)) ? 1LL : 0LL;
      } else {
        tg1_9XMLWriter_writeAttributeNS(rv, ar, count, this_);
      }
    } else {
      throw_wrong_arguments_nr("XMLWriter::writeAttributeNS", count, 4, 4, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("XMLWriter::writeAttributeNS");
  }
  frame_free_locals_inl(ar, 4);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_xmlwriter_write_attribute(Value* xmlwriter, Value* name, Value* value) asm("_ZN4HPHP27f_xmlwriter_write_attributeERKNS_6ObjectERKNS_6StringES5_");

void fg1_xmlwriter_write_attribute(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_xmlwriter_write_attribute(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (!IS_STRING_TYPE((args-2)->m_type)) {
    tvCastToStringInPlace(args-2);
  }
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_xmlwriter_write_attribute(&args[-0].m_data, &args[-1].m_data, &args[-2].m_data)) ? 1LL : 0LL;
}

TypedValue* fg_xmlwriter_write_attribute(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 3) {
    if (IS_STRING_TYPE((args - 2)->m_type) &&
        IS_STRING_TYPE((args - 1)->m_type) &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_xmlwriter_write_attribute(&args[-0].m_data, &args[-1].m_data, &args[-2].m_data)) ? 1LL : 0LL;
    } else {
      fg1_xmlwriter_write_attribute(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("xmlwriter_write_attribute", count, 3, 3, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool th_9XMLWriter_writeAttribute(ObjectData* this_, Value* name, Value* value) asm("_ZN4HPHP11c_XMLWriter16t_writeattributeERKNS_6StringES3_");

void tg1_9XMLWriter_writeAttribute(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) __attribute__((noinline,cold));
void tg1_9XMLWriter_writeAttribute(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (th_9XMLWriter_writeAttribute((this_), &args[-0].m_data, &args[-1].m_data)) ? 1LL : 0LL;
}

TypedValue* tg_9XMLWriter_writeAttribute(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 2) {
      if (IS_STRING_TYPE((args - 1)->m_type) &&
          IS_STRING_TYPE((args - 0)->m_type)) {
        rv->m_type = KindOfBoolean;
        rv->m_data.num = (th_9XMLWriter_writeAttribute((this_), &args[-0].m_data, &args[-1].m_data)) ? 1LL : 0LL;
      } else {
        tg1_9XMLWriter_writeAttribute(rv, ar, count, this_);
      }
    } else {
      throw_wrong_arguments_nr("XMLWriter::writeAttribute", count, 2, 2, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("XMLWriter::writeAttribute");
  }
  frame_free_locals_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_xmlwriter_end_attribute(Value* xmlwriter) asm("_ZN4HPHP25f_xmlwriter_end_attributeERKNS_6ObjectE");

void fg1_xmlwriter_end_attribute(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_xmlwriter_end_attribute(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_xmlwriter_end_attribute(&args[-0].m_data)) ? 1LL : 0LL;
}

TypedValue* fg_xmlwriter_end_attribute(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_xmlwriter_end_attribute(&args[-0].m_data)) ? 1LL : 0LL;
    } else {
      fg1_xmlwriter_end_attribute(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("xmlwriter_end_attribute", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool th_9XMLWriter_endAttribute(ObjectData* this_) asm("_ZN4HPHP11c_XMLWriter14t_endattributeEv");

TypedValue* tg_9XMLWriter_endAttribute(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 0) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (th_9XMLWriter_endAttribute((this_))) ? 1LL : 0LL;
    } else {
      throw_toomany_arguments_nr("XMLWriter::endAttribute", 0, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("XMLWriter::endAttribute");
  }
  frame_free_locals_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_xmlwriter_start_cdata(Value* xmlwriter) asm("_ZN4HPHP23f_xmlwriter_start_cdataERKNS_6ObjectE");

void fg1_xmlwriter_start_cdata(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_xmlwriter_start_cdata(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_xmlwriter_start_cdata(&args[-0].m_data)) ? 1LL : 0LL;
}

TypedValue* fg_xmlwriter_start_cdata(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_xmlwriter_start_cdata(&args[-0].m_data)) ? 1LL : 0LL;
    } else {
      fg1_xmlwriter_start_cdata(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("xmlwriter_start_cdata", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool th_9XMLWriter_startCData(ObjectData* this_) asm("_ZN4HPHP11c_XMLWriter12t_startcdataEv");

TypedValue* tg_9XMLWriter_startCData(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 0) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (th_9XMLWriter_startCData((this_))) ? 1LL : 0LL;
    } else {
      throw_toomany_arguments_nr("XMLWriter::startCData", 0, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("XMLWriter::startCData");
  }
  frame_free_locals_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_xmlwriter_write_cdata(Value* xmlwriter, Value* content) asm("_ZN4HPHP23f_xmlwriter_write_cdataERKNS_6ObjectERKNS_6StringE");

void fg1_xmlwriter_write_cdata(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_xmlwriter_write_cdata(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_xmlwriter_write_cdata(&args[-0].m_data, &args[-1].m_data)) ? 1LL : 0LL;
}

TypedValue* fg_xmlwriter_write_cdata(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if (IS_STRING_TYPE((args - 1)->m_type) &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_xmlwriter_write_cdata(&args[-0].m_data, &args[-1].m_data)) ? 1LL : 0LL;
    } else {
      fg1_xmlwriter_write_cdata(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("xmlwriter_write_cdata", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool th_9XMLWriter_writeCData(ObjectData* this_, Value* content) asm("_ZN4HPHP11c_XMLWriter12t_writecdataERKNS_6StringE");

void tg1_9XMLWriter_writeCData(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) __attribute__((noinline,cold));
void tg1_9XMLWriter_writeCData(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (th_9XMLWriter_writeCData((this_), &args[-0].m_data)) ? 1LL : 0LL;
}

TypedValue* tg_9XMLWriter_writeCData(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 1) {
      if (IS_STRING_TYPE((args - 0)->m_type)) {
        rv->m_type = KindOfBoolean;
        rv->m_data.num = (th_9XMLWriter_writeCData((this_), &args[-0].m_data)) ? 1LL : 0LL;
      } else {
        tg1_9XMLWriter_writeCData(rv, ar, count, this_);
      }
    } else {
      throw_wrong_arguments_nr("XMLWriter::writeCData", count, 1, 1, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("XMLWriter::writeCData");
  }
  frame_free_locals_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_xmlwriter_end_cdata(Value* xmlwriter) asm("_ZN4HPHP21f_xmlwriter_end_cdataERKNS_6ObjectE");

void fg1_xmlwriter_end_cdata(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_xmlwriter_end_cdata(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_xmlwriter_end_cdata(&args[-0].m_data)) ? 1LL : 0LL;
}

TypedValue* fg_xmlwriter_end_cdata(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_xmlwriter_end_cdata(&args[-0].m_data)) ? 1LL : 0LL;
    } else {
      fg1_xmlwriter_end_cdata(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("xmlwriter_end_cdata", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool th_9XMLWriter_endCData(ObjectData* this_) asm("_ZN4HPHP11c_XMLWriter10t_endcdataEv");

TypedValue* tg_9XMLWriter_endCData(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 0) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (th_9XMLWriter_endCData((this_))) ? 1LL : 0LL;
    } else {
      throw_toomany_arguments_nr("XMLWriter::endCData", 0, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("XMLWriter::endCData");
  }
  frame_free_locals_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_xmlwriter_start_comment(Value* xmlwriter) asm("_ZN4HPHP25f_xmlwriter_start_commentERKNS_6ObjectE");

void fg1_xmlwriter_start_comment(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_xmlwriter_start_comment(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_xmlwriter_start_comment(&args[-0].m_data)) ? 1LL : 0LL;
}

TypedValue* fg_xmlwriter_start_comment(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_xmlwriter_start_comment(&args[-0].m_data)) ? 1LL : 0LL;
    } else {
      fg1_xmlwriter_start_comment(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("xmlwriter_start_comment", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool th_9XMLWriter_startComment(ObjectData* this_) asm("_ZN4HPHP11c_XMLWriter14t_startcommentEv");

TypedValue* tg_9XMLWriter_startComment(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 0) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (th_9XMLWriter_startComment((this_))) ? 1LL : 0LL;
    } else {
      throw_toomany_arguments_nr("XMLWriter::startComment", 0, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("XMLWriter::startComment");
  }
  frame_free_locals_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_xmlwriter_write_comment(Value* xmlwriter, Value* content) asm("_ZN4HPHP25f_xmlwriter_write_commentERKNS_6ObjectERKNS_6StringE");

void fg1_xmlwriter_write_comment(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_xmlwriter_write_comment(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_xmlwriter_write_comment(&args[-0].m_data, &args[-1].m_data)) ? 1LL : 0LL;
}

TypedValue* fg_xmlwriter_write_comment(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if (IS_STRING_TYPE((args - 1)->m_type) &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_xmlwriter_write_comment(&args[-0].m_data, &args[-1].m_data)) ? 1LL : 0LL;
    } else {
      fg1_xmlwriter_write_comment(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("xmlwriter_write_comment", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool th_9XMLWriter_writeComment(ObjectData* this_, Value* content) asm("_ZN4HPHP11c_XMLWriter14t_writecommentERKNS_6StringE");

void tg1_9XMLWriter_writeComment(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) __attribute__((noinline,cold));
void tg1_9XMLWriter_writeComment(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (th_9XMLWriter_writeComment((this_), &args[-0].m_data)) ? 1LL : 0LL;
}

TypedValue* tg_9XMLWriter_writeComment(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 1) {
      if (IS_STRING_TYPE((args - 0)->m_type)) {
        rv->m_type = KindOfBoolean;
        rv->m_data.num = (th_9XMLWriter_writeComment((this_), &args[-0].m_data)) ? 1LL : 0LL;
      } else {
        tg1_9XMLWriter_writeComment(rv, ar, count, this_);
      }
    } else {
      throw_wrong_arguments_nr("XMLWriter::writeComment", count, 1, 1, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("XMLWriter::writeComment");
  }
  frame_free_locals_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_xmlwriter_end_comment(Value* xmlwriter) asm("_ZN4HPHP23f_xmlwriter_end_commentERKNS_6ObjectE");

void fg1_xmlwriter_end_comment(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_xmlwriter_end_comment(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_xmlwriter_end_comment(&args[-0].m_data)) ? 1LL : 0LL;
}

TypedValue* fg_xmlwriter_end_comment(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_xmlwriter_end_comment(&args[-0].m_data)) ? 1LL : 0LL;
    } else {
      fg1_xmlwriter_end_comment(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("xmlwriter_end_comment", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool th_9XMLWriter_endComment(ObjectData* this_) asm("_ZN4HPHP11c_XMLWriter12t_endcommentEv");

TypedValue* tg_9XMLWriter_endComment(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 0) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (th_9XMLWriter_endComment((this_))) ? 1LL : 0LL;
    } else {
      throw_toomany_arguments_nr("XMLWriter::endComment", 0, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("XMLWriter::endComment");
  }
  frame_free_locals_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_xmlwriter_end_document(Value* xmlwriter) asm("_ZN4HPHP24f_xmlwriter_end_documentERKNS_6ObjectE");

void fg1_xmlwriter_end_document(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_xmlwriter_end_document(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_xmlwriter_end_document(&args[-0].m_data)) ? 1LL : 0LL;
}

TypedValue* fg_xmlwriter_end_document(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_xmlwriter_end_document(&args[-0].m_data)) ? 1LL : 0LL;
    } else {
      fg1_xmlwriter_end_document(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("xmlwriter_end_document", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool th_9XMLWriter_endDocument(ObjectData* this_) asm("_ZN4HPHP11c_XMLWriter13t_enddocumentEv");

TypedValue* tg_9XMLWriter_endDocument(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 0) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (th_9XMLWriter_endDocument((this_))) ? 1LL : 0LL;
    } else {
      throw_toomany_arguments_nr("XMLWriter::endDocument", 0, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("XMLWriter::endDocument");
  }
  frame_free_locals_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_xmlwriter_start_pi(Value* xmlwriter, Value* target) asm("_ZN4HPHP20f_xmlwriter_start_piERKNS_6ObjectERKNS_6StringE");

void fg1_xmlwriter_start_pi(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_xmlwriter_start_pi(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_xmlwriter_start_pi(&args[-0].m_data, &args[-1].m_data)) ? 1LL : 0LL;
}

TypedValue* fg_xmlwriter_start_pi(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if (IS_STRING_TYPE((args - 1)->m_type) &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_xmlwriter_start_pi(&args[-0].m_data, &args[-1].m_data)) ? 1LL : 0LL;
    } else {
      fg1_xmlwriter_start_pi(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("xmlwriter_start_pi", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool th_9XMLWriter_startPI(ObjectData* this_, Value* target) asm("_ZN4HPHP11c_XMLWriter9t_startpiERKNS_6StringE");

void tg1_9XMLWriter_startPI(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) __attribute__((noinline,cold));
void tg1_9XMLWriter_startPI(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (th_9XMLWriter_startPI((this_), &args[-0].m_data)) ? 1LL : 0LL;
}

TypedValue* tg_9XMLWriter_startPI(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 1) {
      if (IS_STRING_TYPE((args - 0)->m_type)) {
        rv->m_type = KindOfBoolean;
        rv->m_data.num = (th_9XMLWriter_startPI((this_), &args[-0].m_data)) ? 1LL : 0LL;
      } else {
        tg1_9XMLWriter_startPI(rv, ar, count, this_);
      }
    } else {
      throw_wrong_arguments_nr("XMLWriter::startPI", count, 1, 1, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("XMLWriter::startPI");
  }
  frame_free_locals_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_xmlwriter_write_pi(Value* xmlwriter, Value* target, Value* content) asm("_ZN4HPHP20f_xmlwriter_write_piERKNS_6ObjectERKNS_6StringES5_");

void fg1_xmlwriter_write_pi(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_xmlwriter_write_pi(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (!IS_STRING_TYPE((args-2)->m_type)) {
    tvCastToStringInPlace(args-2);
  }
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_xmlwriter_write_pi(&args[-0].m_data, &args[-1].m_data, &args[-2].m_data)) ? 1LL : 0LL;
}

TypedValue* fg_xmlwriter_write_pi(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 3) {
    if (IS_STRING_TYPE((args - 2)->m_type) &&
        IS_STRING_TYPE((args - 1)->m_type) &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_xmlwriter_write_pi(&args[-0].m_data, &args[-1].m_data, &args[-2].m_data)) ? 1LL : 0LL;
    } else {
      fg1_xmlwriter_write_pi(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("xmlwriter_write_pi", count, 3, 3, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool th_9XMLWriter_writePI(ObjectData* this_, Value* target, Value* content) asm("_ZN4HPHP11c_XMLWriter9t_writepiERKNS_6StringES3_");

void tg1_9XMLWriter_writePI(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) __attribute__((noinline,cold));
void tg1_9XMLWriter_writePI(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (th_9XMLWriter_writePI((this_), &args[-0].m_data, &args[-1].m_data)) ? 1LL : 0LL;
}

TypedValue* tg_9XMLWriter_writePI(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 2) {
      if (IS_STRING_TYPE((args - 1)->m_type) &&
          IS_STRING_TYPE((args - 0)->m_type)) {
        rv->m_type = KindOfBoolean;
        rv->m_data.num = (th_9XMLWriter_writePI((this_), &args[-0].m_data, &args[-1].m_data)) ? 1LL : 0LL;
      } else {
        tg1_9XMLWriter_writePI(rv, ar, count, this_);
      }
    } else {
      throw_wrong_arguments_nr("XMLWriter::writePI", count, 2, 2, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("XMLWriter::writePI");
  }
  frame_free_locals_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_xmlwriter_end_pi(Value* xmlwriter) asm("_ZN4HPHP18f_xmlwriter_end_piERKNS_6ObjectE");

void fg1_xmlwriter_end_pi(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_xmlwriter_end_pi(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_xmlwriter_end_pi(&args[-0].m_data)) ? 1LL : 0LL;
}

TypedValue* fg_xmlwriter_end_pi(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_xmlwriter_end_pi(&args[-0].m_data)) ? 1LL : 0LL;
    } else {
      fg1_xmlwriter_end_pi(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("xmlwriter_end_pi", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool th_9XMLWriter_endPI(ObjectData* this_) asm("_ZN4HPHP11c_XMLWriter7t_endpiEv");

TypedValue* tg_9XMLWriter_endPI(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 0) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (th_9XMLWriter_endPI((this_))) ? 1LL : 0LL;
    } else {
      throw_toomany_arguments_nr("XMLWriter::endPI", 0, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("XMLWriter::endPI");
  }
  frame_free_locals_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_xmlwriter_text(Value* xmlwriter, Value* content) asm("_ZN4HPHP16f_xmlwriter_textERKNS_6ObjectERKNS_6StringE");

void fg1_xmlwriter_text(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_xmlwriter_text(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_xmlwriter_text(&args[-0].m_data, &args[-1].m_data)) ? 1LL : 0LL;
}

TypedValue* fg_xmlwriter_text(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if (IS_STRING_TYPE((args - 1)->m_type) &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_xmlwriter_text(&args[-0].m_data, &args[-1].m_data)) ? 1LL : 0LL;
    } else {
      fg1_xmlwriter_text(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("xmlwriter_text", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool th_9XMLWriter_text(ObjectData* this_, Value* content) asm("_ZN4HPHP11c_XMLWriter6t_textERKNS_6StringE");

void tg1_9XMLWriter_text(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) __attribute__((noinline,cold));
void tg1_9XMLWriter_text(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (th_9XMLWriter_text((this_), &args[-0].m_data)) ? 1LL : 0LL;
}

TypedValue* tg_9XMLWriter_text(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 1) {
      if (IS_STRING_TYPE((args - 0)->m_type)) {
        rv->m_type = KindOfBoolean;
        rv->m_data.num = (th_9XMLWriter_text((this_), &args[-0].m_data)) ? 1LL : 0LL;
      } else {
        tg1_9XMLWriter_text(rv, ar, count, this_);
      }
    } else {
      throw_wrong_arguments_nr("XMLWriter::text", count, 1, 1, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("XMLWriter::text");
  }
  frame_free_locals_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_xmlwriter_write_raw(Value* xmlwriter, Value* content) asm("_ZN4HPHP21f_xmlwriter_write_rawERKNS_6ObjectERKNS_6StringE");

void fg1_xmlwriter_write_raw(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_xmlwriter_write_raw(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_xmlwriter_write_raw(&args[-0].m_data, &args[-1].m_data)) ? 1LL : 0LL;
}

TypedValue* fg_xmlwriter_write_raw(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if (IS_STRING_TYPE((args - 1)->m_type) &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_xmlwriter_write_raw(&args[-0].m_data, &args[-1].m_data)) ? 1LL : 0LL;
    } else {
      fg1_xmlwriter_write_raw(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("xmlwriter_write_raw", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool th_9XMLWriter_writeRaw(ObjectData* this_, Value* content) asm("_ZN4HPHP11c_XMLWriter10t_writerawERKNS_6StringE");

void tg1_9XMLWriter_writeRaw(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) __attribute__((noinline,cold));
void tg1_9XMLWriter_writeRaw(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (th_9XMLWriter_writeRaw((this_), &args[-0].m_data)) ? 1LL : 0LL;
}

TypedValue* tg_9XMLWriter_writeRaw(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 1) {
      if (IS_STRING_TYPE((args - 0)->m_type)) {
        rv->m_type = KindOfBoolean;
        rv->m_data.num = (th_9XMLWriter_writeRaw((this_), &args[-0].m_data)) ? 1LL : 0LL;
      } else {
        tg1_9XMLWriter_writeRaw(rv, ar, count, this_);
      }
    } else {
      throw_wrong_arguments_nr("XMLWriter::writeRaw", count, 1, 1, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("XMLWriter::writeRaw");
  }
  frame_free_locals_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_xmlwriter_start_dtd(Value* xmlwriter, Value* qualifiedname, Value* publicid, Value* systemid) asm("_ZN4HPHP21f_xmlwriter_start_dtdERKNS_6ObjectERKNS_6StringES5_S5_");

void fg1_xmlwriter_start_dtd(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_xmlwriter_start_dtd(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 4
    if (!IS_STRING_TYPE((args-3)->m_type)) {
      tvCastToStringInPlace(args-3);
    }
  case 3:
    if (!IS_STRING_TYPE((args-2)->m_type)) {
      tvCastToStringInPlace(args-2);
    }
  case 2:
    break;
  }
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_xmlwriter_start_dtd(&args[-0].m_data, &args[-1].m_data, (count > 2) ? &args[-2].m_data : (Value*)(&null_string), (count > 3) ? &args[-3].m_data : (Value*)(&null_string))) ? 1LL : 0LL;
}

TypedValue* fg_xmlwriter_start_dtd(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 2 && count <= 4) {
    if ((count <= 3 || IS_STRING_TYPE((args - 3)->m_type)) &&
        (count <= 2 || IS_STRING_TYPE((args - 2)->m_type)) &&
        IS_STRING_TYPE((args - 1)->m_type) &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_xmlwriter_start_dtd(&args[-0].m_data, &args[-1].m_data, (count > 2) ? &args[-2].m_data : (Value*)(&null_string), (count > 3) ? &args[-3].m_data : (Value*)(&null_string))) ? 1LL : 0LL;
    } else {
      fg1_xmlwriter_start_dtd(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("xmlwriter_start_dtd", count, 2, 4, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 4);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool th_9XMLWriter_startDTD(ObjectData* this_, Value* qualifiedname, Value* publicid, Value* systemid) asm("_ZN4HPHP11c_XMLWriter10t_startdtdERKNS_6StringES3_S3_");

void tg1_9XMLWriter_startDTD(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) __attribute__((noinline,cold));
void tg1_9XMLWriter_startDTD(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 3
    if (!IS_STRING_TYPE((args-2)->m_type)) {
      tvCastToStringInPlace(args-2);
    }
  case 2:
    if (!IS_STRING_TYPE((args-1)->m_type)) {
      tvCastToStringInPlace(args-1);
    }
  case 1:
    break;
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (th_9XMLWriter_startDTD((this_), &args[-0].m_data, (count > 1) ? &args[-1].m_data : (Value*)(&null_string), (count > 2) ? &args[-2].m_data : (Value*)(&null_string))) ? 1LL : 0LL;
}

TypedValue* tg_9XMLWriter_startDTD(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count >= 1 && count <= 3) {
      if ((count <= 2 || IS_STRING_TYPE((args - 2)->m_type)) &&
          (count <= 1 || IS_STRING_TYPE((args - 1)->m_type)) &&
          IS_STRING_TYPE((args - 0)->m_type)) {
        rv->m_type = KindOfBoolean;
        rv->m_data.num = (th_9XMLWriter_startDTD((this_), &args[-0].m_data, (count > 1) ? &args[-1].m_data : (Value*)(&null_string), (count > 2) ? &args[-2].m_data : (Value*)(&null_string))) ? 1LL : 0LL;
      } else {
        tg1_9XMLWriter_startDTD(rv, ar, count, this_);
      }
    } else {
      throw_wrong_arguments_nr("XMLWriter::startDTD", count, 1, 3, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("XMLWriter::startDTD");
  }
  frame_free_locals_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_xmlwriter_write_dtd(Value* xmlwriter, Value* name, Value* publicid, Value* systemid, Value* subset) asm("_ZN4HPHP21f_xmlwriter_write_dtdERKNS_6ObjectERKNS_6StringES5_S5_S5_");

void fg1_xmlwriter_write_dtd(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_xmlwriter_write_dtd(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 5
    if (!IS_STRING_TYPE((args-4)->m_type)) {
      tvCastToStringInPlace(args-4);
    }
  case 4:
    if (!IS_STRING_TYPE((args-3)->m_type)) {
      tvCastToStringInPlace(args-3);
    }
  case 3:
    if (!IS_STRING_TYPE((args-2)->m_type)) {
      tvCastToStringInPlace(args-2);
    }
  case 2:
    break;
  }
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_xmlwriter_write_dtd(&args[-0].m_data, &args[-1].m_data, (count > 2) ? &args[-2].m_data : (Value*)(&null_string), (count > 3) ? &args[-3].m_data : (Value*)(&null_string), (count > 4) ? &args[-4].m_data : (Value*)(&null_string))) ? 1LL : 0LL;
}

TypedValue* fg_xmlwriter_write_dtd(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 2 && count <= 5) {
    if ((count <= 4 || IS_STRING_TYPE((args - 4)->m_type)) &&
        (count <= 3 || IS_STRING_TYPE((args - 3)->m_type)) &&
        (count <= 2 || IS_STRING_TYPE((args - 2)->m_type)) &&
        IS_STRING_TYPE((args - 1)->m_type) &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_xmlwriter_write_dtd(&args[-0].m_data, &args[-1].m_data, (count > 2) ? &args[-2].m_data : (Value*)(&null_string), (count > 3) ? &args[-3].m_data : (Value*)(&null_string), (count > 4) ? &args[-4].m_data : (Value*)(&null_string))) ? 1LL : 0LL;
    } else {
      fg1_xmlwriter_write_dtd(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("xmlwriter_write_dtd", count, 2, 5, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 5);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool th_9XMLWriter_writeDTD(ObjectData* this_, Value* name, Value* publicid, Value* systemid, Value* subset) asm("_ZN4HPHP11c_XMLWriter10t_writedtdERKNS_6StringES3_S3_S3_");

void tg1_9XMLWriter_writeDTD(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) __attribute__((noinline,cold));
void tg1_9XMLWriter_writeDTD(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 4
    if (!IS_STRING_TYPE((args-3)->m_type)) {
      tvCastToStringInPlace(args-3);
    }
  case 3:
    if (!IS_STRING_TYPE((args-2)->m_type)) {
      tvCastToStringInPlace(args-2);
    }
  case 2:
    if (!IS_STRING_TYPE((args-1)->m_type)) {
      tvCastToStringInPlace(args-1);
    }
  case 1:
    break;
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (th_9XMLWriter_writeDTD((this_), &args[-0].m_data, (count > 1) ? &args[-1].m_data : (Value*)(&null_string), (count > 2) ? &args[-2].m_data : (Value*)(&null_string), (count > 3) ? &args[-3].m_data : (Value*)(&null_string))) ? 1LL : 0LL;
}

TypedValue* tg_9XMLWriter_writeDTD(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count >= 1 && count <= 4) {
      if ((count <= 3 || IS_STRING_TYPE((args - 3)->m_type)) &&
          (count <= 2 || IS_STRING_TYPE((args - 2)->m_type)) &&
          (count <= 1 || IS_STRING_TYPE((args - 1)->m_type)) &&
          IS_STRING_TYPE((args - 0)->m_type)) {
        rv->m_type = KindOfBoolean;
        rv->m_data.num = (th_9XMLWriter_writeDTD((this_), &args[-0].m_data, (count > 1) ? &args[-1].m_data : (Value*)(&null_string), (count > 2) ? &args[-2].m_data : (Value*)(&null_string), (count > 3) ? &args[-3].m_data : (Value*)(&null_string))) ? 1LL : 0LL;
      } else {
        tg1_9XMLWriter_writeDTD(rv, ar, count, this_);
      }
    } else {
      throw_wrong_arguments_nr("XMLWriter::writeDTD", count, 1, 4, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("XMLWriter::writeDTD");
  }
  frame_free_locals_inl(ar, 4);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_xmlwriter_start_dtd_element(Value* xmlwriter, Value* qualifiedname) asm("_ZN4HPHP29f_xmlwriter_start_dtd_elementERKNS_6ObjectERKNS_6StringE");

void fg1_xmlwriter_start_dtd_element(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_xmlwriter_start_dtd_element(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_xmlwriter_start_dtd_element(&args[-0].m_data, &args[-1].m_data)) ? 1LL : 0LL;
}

TypedValue* fg_xmlwriter_start_dtd_element(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if (IS_STRING_TYPE((args - 1)->m_type) &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_xmlwriter_start_dtd_element(&args[-0].m_data, &args[-1].m_data)) ? 1LL : 0LL;
    } else {
      fg1_xmlwriter_start_dtd_element(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("xmlwriter_start_dtd_element", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool th_9XMLWriter_startDTDElement(ObjectData* this_, Value* qualifiedname) asm("_ZN4HPHP11c_XMLWriter17t_startdtdelementERKNS_6StringE");

void tg1_9XMLWriter_startDTDElement(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) __attribute__((noinline,cold));
void tg1_9XMLWriter_startDTDElement(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (th_9XMLWriter_startDTDElement((this_), &args[-0].m_data)) ? 1LL : 0LL;
}

TypedValue* tg_9XMLWriter_startDTDElement(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 1) {
      if (IS_STRING_TYPE((args - 0)->m_type)) {
        rv->m_type = KindOfBoolean;
        rv->m_data.num = (th_9XMLWriter_startDTDElement((this_), &args[-0].m_data)) ? 1LL : 0LL;
      } else {
        tg1_9XMLWriter_startDTDElement(rv, ar, count, this_);
      }
    } else {
      throw_wrong_arguments_nr("XMLWriter::startDTDElement", count, 1, 1, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("XMLWriter::startDTDElement");
  }
  frame_free_locals_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_xmlwriter_write_dtd_element(Value* xmlwriter, Value* name, Value* content) asm("_ZN4HPHP29f_xmlwriter_write_dtd_elementERKNS_6ObjectERKNS_6StringES5_");

void fg1_xmlwriter_write_dtd_element(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_xmlwriter_write_dtd_element(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (!IS_STRING_TYPE((args-2)->m_type)) {
    tvCastToStringInPlace(args-2);
  }
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_xmlwriter_write_dtd_element(&args[-0].m_data, &args[-1].m_data, &args[-2].m_data)) ? 1LL : 0LL;
}

TypedValue* fg_xmlwriter_write_dtd_element(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 3) {
    if (IS_STRING_TYPE((args - 2)->m_type) &&
        IS_STRING_TYPE((args - 1)->m_type) &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_xmlwriter_write_dtd_element(&args[-0].m_data, &args[-1].m_data, &args[-2].m_data)) ? 1LL : 0LL;
    } else {
      fg1_xmlwriter_write_dtd_element(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("xmlwriter_write_dtd_element", count, 3, 3, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool th_9XMLWriter_writeDTDElement(ObjectData* this_, Value* name, Value* content) asm("_ZN4HPHP11c_XMLWriter17t_writedtdelementERKNS_6StringES3_");

void tg1_9XMLWriter_writeDTDElement(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) __attribute__((noinline,cold));
void tg1_9XMLWriter_writeDTDElement(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (th_9XMLWriter_writeDTDElement((this_), &args[-0].m_data, &args[-1].m_data)) ? 1LL : 0LL;
}

TypedValue* tg_9XMLWriter_writeDTDElement(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 2) {
      if (IS_STRING_TYPE((args - 1)->m_type) &&
          IS_STRING_TYPE((args - 0)->m_type)) {
        rv->m_type = KindOfBoolean;
        rv->m_data.num = (th_9XMLWriter_writeDTDElement((this_), &args[-0].m_data, &args[-1].m_data)) ? 1LL : 0LL;
      } else {
        tg1_9XMLWriter_writeDTDElement(rv, ar, count, this_);
      }
    } else {
      throw_wrong_arguments_nr("XMLWriter::writeDTDElement", count, 2, 2, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("XMLWriter::writeDTDElement");
  }
  frame_free_locals_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_xmlwriter_end_dtd_element(Value* xmlwriter) asm("_ZN4HPHP27f_xmlwriter_end_dtd_elementERKNS_6ObjectE");

void fg1_xmlwriter_end_dtd_element(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_xmlwriter_end_dtd_element(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_xmlwriter_end_dtd_element(&args[-0].m_data)) ? 1LL : 0LL;
}

TypedValue* fg_xmlwriter_end_dtd_element(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_xmlwriter_end_dtd_element(&args[-0].m_data)) ? 1LL : 0LL;
    } else {
      fg1_xmlwriter_end_dtd_element(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("xmlwriter_end_dtd_element", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool th_9XMLWriter_endDTDElement(ObjectData* this_) asm("_ZN4HPHP11c_XMLWriter15t_enddtdelementEv");

TypedValue* tg_9XMLWriter_endDTDElement(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 0) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (th_9XMLWriter_endDTDElement((this_))) ? 1LL : 0LL;
    } else {
      throw_toomany_arguments_nr("XMLWriter::endDTDElement", 0, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("XMLWriter::endDTDElement");
  }
  frame_free_locals_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_xmlwriter_start_dtd_attlist(Value* xmlwriter, Value* name) asm("_ZN4HPHP29f_xmlwriter_start_dtd_attlistERKNS_6ObjectERKNS_6StringE");

void fg1_xmlwriter_start_dtd_attlist(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_xmlwriter_start_dtd_attlist(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_xmlwriter_start_dtd_attlist(&args[-0].m_data, &args[-1].m_data)) ? 1LL : 0LL;
}

TypedValue* fg_xmlwriter_start_dtd_attlist(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if (IS_STRING_TYPE((args - 1)->m_type) &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_xmlwriter_start_dtd_attlist(&args[-0].m_data, &args[-1].m_data)) ? 1LL : 0LL;
    } else {
      fg1_xmlwriter_start_dtd_attlist(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("xmlwriter_start_dtd_attlist", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool th_9XMLWriter_startDTDAttlist(ObjectData* this_, Value* name) asm("_ZN4HPHP11c_XMLWriter17t_startdtdattlistERKNS_6StringE");

void tg1_9XMLWriter_startDTDAttlist(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) __attribute__((noinline,cold));
void tg1_9XMLWriter_startDTDAttlist(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (th_9XMLWriter_startDTDAttlist((this_), &args[-0].m_data)) ? 1LL : 0LL;
}

TypedValue* tg_9XMLWriter_startDTDAttlist(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 1) {
      if (IS_STRING_TYPE((args - 0)->m_type)) {
        rv->m_type = KindOfBoolean;
        rv->m_data.num = (th_9XMLWriter_startDTDAttlist((this_), &args[-0].m_data)) ? 1LL : 0LL;
      } else {
        tg1_9XMLWriter_startDTDAttlist(rv, ar, count, this_);
      }
    } else {
      throw_wrong_arguments_nr("XMLWriter::startDTDAttlist", count, 1, 1, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("XMLWriter::startDTDAttlist");
  }
  frame_free_locals_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_xmlwriter_write_dtd_attlist(Value* xmlwriter, Value* name, Value* content) asm("_ZN4HPHP29f_xmlwriter_write_dtd_attlistERKNS_6ObjectERKNS_6StringES5_");

void fg1_xmlwriter_write_dtd_attlist(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_xmlwriter_write_dtd_attlist(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (!IS_STRING_TYPE((args-2)->m_type)) {
    tvCastToStringInPlace(args-2);
  }
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_xmlwriter_write_dtd_attlist(&args[-0].m_data, &args[-1].m_data, &args[-2].m_data)) ? 1LL : 0LL;
}

TypedValue* fg_xmlwriter_write_dtd_attlist(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 3) {
    if (IS_STRING_TYPE((args - 2)->m_type) &&
        IS_STRING_TYPE((args - 1)->m_type) &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_xmlwriter_write_dtd_attlist(&args[-0].m_data, &args[-1].m_data, &args[-2].m_data)) ? 1LL : 0LL;
    } else {
      fg1_xmlwriter_write_dtd_attlist(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("xmlwriter_write_dtd_attlist", count, 3, 3, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool th_9XMLWriter_writeDTDAttlist(ObjectData* this_, Value* name, Value* content) asm("_ZN4HPHP11c_XMLWriter17t_writedtdattlistERKNS_6StringES3_");

void tg1_9XMLWriter_writeDTDAttlist(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) __attribute__((noinline,cold));
void tg1_9XMLWriter_writeDTDAttlist(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (th_9XMLWriter_writeDTDAttlist((this_), &args[-0].m_data, &args[-1].m_data)) ? 1LL : 0LL;
}

TypedValue* tg_9XMLWriter_writeDTDAttlist(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 2) {
      if (IS_STRING_TYPE((args - 1)->m_type) &&
          IS_STRING_TYPE((args - 0)->m_type)) {
        rv->m_type = KindOfBoolean;
        rv->m_data.num = (th_9XMLWriter_writeDTDAttlist((this_), &args[-0].m_data, &args[-1].m_data)) ? 1LL : 0LL;
      } else {
        tg1_9XMLWriter_writeDTDAttlist(rv, ar, count, this_);
      }
    } else {
      throw_wrong_arguments_nr("XMLWriter::writeDTDAttlist", count, 2, 2, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("XMLWriter::writeDTDAttlist");
  }
  frame_free_locals_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_xmlwriter_end_dtd_attlist(Value* xmlwriter) asm("_ZN4HPHP27f_xmlwriter_end_dtd_attlistERKNS_6ObjectE");

void fg1_xmlwriter_end_dtd_attlist(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_xmlwriter_end_dtd_attlist(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_xmlwriter_end_dtd_attlist(&args[-0].m_data)) ? 1LL : 0LL;
}

TypedValue* fg_xmlwriter_end_dtd_attlist(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_xmlwriter_end_dtd_attlist(&args[-0].m_data)) ? 1LL : 0LL;
    } else {
      fg1_xmlwriter_end_dtd_attlist(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("xmlwriter_end_dtd_attlist", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool th_9XMLWriter_endDTDAttlist(ObjectData* this_) asm("_ZN4HPHP11c_XMLWriter15t_enddtdattlistEv");

TypedValue* tg_9XMLWriter_endDTDAttlist(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 0) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (th_9XMLWriter_endDTDAttlist((this_))) ? 1LL : 0LL;
    } else {
      throw_toomany_arguments_nr("XMLWriter::endDTDAttlist", 0, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("XMLWriter::endDTDAttlist");
  }
  frame_free_locals_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_xmlwriter_start_dtd_entity(Value* xmlwriter, Value* name, bool isparam) asm("_ZN4HPHP28f_xmlwriter_start_dtd_entityERKNS_6ObjectERKNS_6StringEb");

void fg1_xmlwriter_start_dtd_entity(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_xmlwriter_start_dtd_entity(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-2)->m_type != KindOfBoolean) {
    tvCastToBooleanInPlace(args-2);
  }
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_xmlwriter_start_dtd_entity(&args[-0].m_data, &args[-1].m_data, (bool)(args[-2].m_data.num))) ? 1LL : 0LL;
}

TypedValue* fg_xmlwriter_start_dtd_entity(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 3) {
    if ((args - 2)->m_type == KindOfBoolean &&
        IS_STRING_TYPE((args - 1)->m_type) &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_xmlwriter_start_dtd_entity(&args[-0].m_data, &args[-1].m_data, (bool)(args[-2].m_data.num))) ? 1LL : 0LL;
    } else {
      fg1_xmlwriter_start_dtd_entity(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("xmlwriter_start_dtd_entity", count, 3, 3, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool th_9XMLWriter_startDTDEntity(ObjectData* this_, Value* name, bool isparam) asm("_ZN4HPHP11c_XMLWriter16t_startdtdentityERKNS_6StringEb");

void tg1_9XMLWriter_startDTDEntity(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) __attribute__((noinline,cold));
void tg1_9XMLWriter_startDTDEntity(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-1)->m_type != KindOfBoolean) {
    tvCastToBooleanInPlace(args-1);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (th_9XMLWriter_startDTDEntity((this_), &args[-0].m_data, (bool)(args[-1].m_data.num))) ? 1LL : 0LL;
}

TypedValue* tg_9XMLWriter_startDTDEntity(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 2) {
      if ((args - 1)->m_type == KindOfBoolean &&
          IS_STRING_TYPE((args - 0)->m_type)) {
        rv->m_type = KindOfBoolean;
        rv->m_data.num = (th_9XMLWriter_startDTDEntity((this_), &args[-0].m_data, (bool)(args[-1].m_data.num))) ? 1LL : 0LL;
      } else {
        tg1_9XMLWriter_startDTDEntity(rv, ar, count, this_);
      }
    } else {
      throw_wrong_arguments_nr("XMLWriter::startDTDEntity", count, 2, 2, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("XMLWriter::startDTDEntity");
  }
  frame_free_locals_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_xmlwriter_write_dtd_entity(Value* xmlwriter, Value* name, Value* content, bool pe, Value* publicid, Value* systemid, Value* ndataid) asm("_ZN4HPHP28f_xmlwriter_write_dtd_entityERKNS_6ObjectERKNS_6StringES5_bS5_S5_S5_");

void fg1_xmlwriter_write_dtd_entity(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_xmlwriter_write_dtd_entity(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 7
    if (!IS_STRING_TYPE((args-6)->m_type)) {
      tvCastToStringInPlace(args-6);
    }
  case 6:
    if (!IS_STRING_TYPE((args-5)->m_type)) {
      tvCastToStringInPlace(args-5);
    }
  case 5:
    if (!IS_STRING_TYPE((args-4)->m_type)) {
      tvCastToStringInPlace(args-4);
    }
  case 4:
    if ((args-3)->m_type != KindOfBoolean) {
      tvCastToBooleanInPlace(args-3);
    }
  case 3:
    break;
  }
  if (!IS_STRING_TYPE((args-2)->m_type)) {
    tvCastToStringInPlace(args-2);
  }
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_xmlwriter_write_dtd_entity(&args[-0].m_data, &args[-1].m_data, &args[-2].m_data, (count > 3) ? (bool)(args[-3].m_data.num) : (bool)(false), (count > 4) ? &args[-4].m_data : (Value*)(&null_string), (count > 5) ? &args[-5].m_data : (Value*)(&null_string), (count > 6) ? &args[-6].m_data : (Value*)(&null_string))) ? 1LL : 0LL;
}

TypedValue* fg_xmlwriter_write_dtd_entity(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 3 && count <= 7) {
    if ((count <= 6 || IS_STRING_TYPE((args - 6)->m_type)) &&
        (count <= 5 || IS_STRING_TYPE((args - 5)->m_type)) &&
        (count <= 4 || IS_STRING_TYPE((args - 4)->m_type)) &&
        (count <= 3 || (args - 3)->m_type == KindOfBoolean) &&
        IS_STRING_TYPE((args - 2)->m_type) &&
        IS_STRING_TYPE((args - 1)->m_type) &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_xmlwriter_write_dtd_entity(&args[-0].m_data, &args[-1].m_data, &args[-2].m_data, (count > 3) ? (bool)(args[-3].m_data.num) : (bool)(false), (count > 4) ? &args[-4].m_data : (Value*)(&null_string), (count > 5) ? &args[-5].m_data : (Value*)(&null_string), (count > 6) ? &args[-6].m_data : (Value*)(&null_string))) ? 1LL : 0LL;
    } else {
      fg1_xmlwriter_write_dtd_entity(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("xmlwriter_write_dtd_entity", count, 3, 7, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 7);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool th_9XMLWriter_writeDTDEntity(ObjectData* this_, Value* name, Value* content, bool pe, Value* publicid, Value* systemid, Value* ndataid) asm("_ZN4HPHP11c_XMLWriter16t_writedtdentityERKNS_6StringES3_bS3_S3_S3_");

void tg1_9XMLWriter_writeDTDEntity(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) __attribute__((noinline,cold));
void tg1_9XMLWriter_writeDTDEntity(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 6
    if (!IS_STRING_TYPE((args-5)->m_type)) {
      tvCastToStringInPlace(args-5);
    }
  case 5:
    if (!IS_STRING_TYPE((args-4)->m_type)) {
      tvCastToStringInPlace(args-4);
    }
  case 4:
    if (!IS_STRING_TYPE((args-3)->m_type)) {
      tvCastToStringInPlace(args-3);
    }
  case 3:
    if ((args-2)->m_type != KindOfBoolean) {
      tvCastToBooleanInPlace(args-2);
    }
  case 2:
    break;
  }
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (th_9XMLWriter_writeDTDEntity((this_), &args[-0].m_data, &args[-1].m_data, (count > 2) ? (bool)(args[-2].m_data.num) : (bool)(false), (count > 3) ? &args[-3].m_data : (Value*)(&null_string), (count > 4) ? &args[-4].m_data : (Value*)(&null_string), (count > 5) ? &args[-5].m_data : (Value*)(&null_string))) ? 1LL : 0LL;
}

TypedValue* tg_9XMLWriter_writeDTDEntity(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count >= 2 && count <= 6) {
      if ((count <= 5 || IS_STRING_TYPE((args - 5)->m_type)) &&
          (count <= 4 || IS_STRING_TYPE((args - 4)->m_type)) &&
          (count <= 3 || IS_STRING_TYPE((args - 3)->m_type)) &&
          (count <= 2 || (args - 2)->m_type == KindOfBoolean) &&
          IS_STRING_TYPE((args - 1)->m_type) &&
          IS_STRING_TYPE((args - 0)->m_type)) {
        rv->m_type = KindOfBoolean;
        rv->m_data.num = (th_9XMLWriter_writeDTDEntity((this_), &args[-0].m_data, &args[-1].m_data, (count > 2) ? (bool)(args[-2].m_data.num) : (bool)(false), (count > 3) ? &args[-3].m_data : (Value*)(&null_string), (count > 4) ? &args[-4].m_data : (Value*)(&null_string), (count > 5) ? &args[-5].m_data : (Value*)(&null_string))) ? 1LL : 0LL;
      } else {
        tg1_9XMLWriter_writeDTDEntity(rv, ar, count, this_);
      }
    } else {
      throw_wrong_arguments_nr("XMLWriter::writeDTDEntity", count, 2, 6, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("XMLWriter::writeDTDEntity");
  }
  frame_free_locals_inl(ar, 6);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_xmlwriter_end_dtd_entity(Value* xmlwriter) asm("_ZN4HPHP26f_xmlwriter_end_dtd_entityERKNS_6ObjectE");

void fg1_xmlwriter_end_dtd_entity(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_xmlwriter_end_dtd_entity(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_xmlwriter_end_dtd_entity(&args[-0].m_data)) ? 1LL : 0LL;
}

TypedValue* fg_xmlwriter_end_dtd_entity(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_xmlwriter_end_dtd_entity(&args[-0].m_data)) ? 1LL : 0LL;
    } else {
      fg1_xmlwriter_end_dtd_entity(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("xmlwriter_end_dtd_entity", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool th_9XMLWriter_endDTDEntity(ObjectData* this_) asm("_ZN4HPHP11c_XMLWriter14t_enddtdentityEv");

TypedValue* tg_9XMLWriter_endDTDEntity(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 0) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (th_9XMLWriter_endDTDEntity((this_))) ? 1LL : 0LL;
    } else {
      throw_toomany_arguments_nr("XMLWriter::endDTDEntity", 0, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("XMLWriter::endDTDEntity");
  }
  frame_free_locals_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_xmlwriter_end_dtd(Value* xmlwriter) asm("_ZN4HPHP19f_xmlwriter_end_dtdERKNS_6ObjectE");

void fg1_xmlwriter_end_dtd(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_xmlwriter_end_dtd(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_xmlwriter_end_dtd(&args[-0].m_data)) ? 1LL : 0LL;
}

TypedValue* fg_xmlwriter_end_dtd(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_xmlwriter_end_dtd(&args[-0].m_data)) ? 1LL : 0LL;
    } else {
      fg1_xmlwriter_end_dtd(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("xmlwriter_end_dtd", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool th_9XMLWriter_endDTD(ObjectData* this_) asm("_ZN4HPHP11c_XMLWriter8t_enddtdEv");

TypedValue* tg_9XMLWriter_endDTD(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 0) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (th_9XMLWriter_endDTD((this_))) ? 1LL : 0LL;
    } else {
      throw_toomany_arguments_nr("XMLWriter::endDTD", 0, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("XMLWriter::endDTD");
  }
  frame_free_locals_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_xmlwriter_flush(TypedValue* _rv, Value* xmlwriter, bool empty) asm("_ZN4HPHP17f_xmlwriter_flushERKNS_6ObjectEb");

void fg1_xmlwriter_flush(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_xmlwriter_flush(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 2
    if ((args-1)->m_type != KindOfBoolean) {
      tvCastToBooleanInPlace(args-1);
    }
  case 1:
    break;
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_xmlwriter_flush(rv, &args[-0].m_data, (count > 1) ? (bool)(args[-1].m_data.num) : (bool)(true));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_xmlwriter_flush(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 1 && count <= 2) {
    if ((count <= 1 || (args - 1)->m_type == KindOfBoolean) &&
        (args - 0)->m_type == KindOfObject) {
      fh_xmlwriter_flush(rv, &args[-0].m_data, (count > 1) ? (bool)(args[-1].m_data.num) : (bool)(true));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_xmlwriter_flush(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("xmlwriter_flush", count, 1, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* th_9XMLWriter_flush(TypedValue* _rv, ObjectData* this_, bool empty) asm("_ZN4HPHP11c_XMLWriter7t_flushEb");

void tg1_9XMLWriter_flush(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) __attribute__((noinline,cold));
void tg1_9XMLWriter_flush(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToBooleanInPlace(args-0);
  th_9XMLWriter_flush(rv, (this_), (count > 0) ? (bool)(args[-0].m_data.num) : (bool)(true));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* tg_9XMLWriter_flush(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count <= 1) {
      if ((count <= 0 || (args - 0)->m_type == KindOfBoolean)) {
        th_9XMLWriter_flush(rv, (this_), (count > 0) ? (bool)(args[-0].m_data.num) : (bool)(true));
        if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
      } else {
        tg1_9XMLWriter_flush(rv, ar, count, this_);
      }
    } else {
      throw_toomany_arguments_nr("XMLWriter::flush", 1, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("XMLWriter::flush");
  }
  frame_free_locals_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_xmlwriter_output_memory(Value* _rv, Value* xmlwriter, bool flush) asm("_ZN4HPHP25f_xmlwriter_output_memoryERKNS_6ObjectEb");

void fg1_xmlwriter_output_memory(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_xmlwriter_output_memory(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 2
    if ((args-1)->m_type != KindOfBoolean) {
      tvCastToBooleanInPlace(args-1);
    }
  case 1:
    break;
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfString;
  fh_xmlwriter_output_memory(&(rv->m_data), &args[-0].m_data, (count > 1) ? (bool)(args[-1].m_data.num) : (bool)(true));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
}

TypedValue* fg_xmlwriter_output_memory(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 1 && count <= 2) {
    if ((count <= 1 || (args - 1)->m_type == KindOfBoolean) &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfString;
      fh_xmlwriter_output_memory(&(rv->m_data), &args[-0].m_data, (count > 1) ? (bool)(args[-1].m_data.num) : (bool)(true));
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      fg1_xmlwriter_output_memory(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("xmlwriter_output_memory", count, 1, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* th_9XMLWriter_outputMemory(Value* _rv, ObjectData* this_, bool flush) asm("_ZN4HPHP11c_XMLWriter14t_outputmemoryEb");

void tg1_9XMLWriter_outputMemory(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) __attribute__((noinline,cold));
void tg1_9XMLWriter_outputMemory(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToBooleanInPlace(args-0);
  rv->m_type = KindOfString;
  th_9XMLWriter_outputMemory(&(rv->m_data), (this_), (count > 0) ? (bool)(args[-0].m_data.num) : (bool)(true));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
}

TypedValue* tg_9XMLWriter_outputMemory(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count <= 1) {
      if ((count <= 0 || (args - 0)->m_type == KindOfBoolean)) {
        rv->m_type = KindOfString;
        th_9XMLWriter_outputMemory(&(rv->m_data), (this_), (count > 0) ? (bool)(args[-0].m_data.num) : (bool)(true));
        if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
      } else {
        tg1_9XMLWriter_outputMemory(rv, ar, count, this_);
      }
    } else {
      throw_toomany_arguments_nr("XMLWriter::outputMemory", 1, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("XMLWriter::outputMemory");
  }
  frame_free_locals_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}


HPHP::VM::Instance* new_XMLWriter_Instance(HPHP::VM::Class* cls) {
  size_t nProps = cls->numDeclProperties();
  size_t builtinPropSize = sizeof(c_XMLWriter) - sizeof(ObjectData);
  size_t size = HPHP::VM::Instance::sizeForNProps(nProps) + builtinPropSize;
  HPHP::VM::Instance *inst = (HPHP::VM::Instance*)ALLOCOBJSZ(size);
  new ((void *)inst) c_XMLWriter(cls);
  return inst;
}

IMPLEMENT_CLASS(XMLWriter);
void th_9XMLWriter___construct(ObjectData* this_) asm("_ZN4HPHP11c_XMLWriter13t___constructEv");

TypedValue* tg_9XMLWriter___construct(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 0) {
      rv->m_type = KindOfNull;
      th_9XMLWriter___construct((this_));
    } else {
      throw_toomany_arguments_nr("XMLWriter::__construct", 0, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("XMLWriter::__construct");
  }
  frame_free_locals_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

} // namespace HPHP
