#include "hphp/runtime/ext/std/ext_std_function.h"
#include "hphp/runtime/ext/std/ext_std_string.h"
#include "hphp/runtime/ext/std/ext_std_classobj.h"
#include "hphp/runtime/ext/reflection/ext_reflection.h"
#include "hphp/util/exception.h"
#include <stdint.h>
#include <sstream>
#include "php_protobuf.h"
#include "ext_protobuf.h"
#include "reader.h"
#include "writer.h"

namespace HPHP {

#define PB_RESET_METHOD "reset"
#define PB_DUMP_METHOD "dump"
#define PB_FIELDS_METHOD "fields"
#define PB_PARSE_FROM_STRING_METHOD "parseFromString"
#define PB_SERIALIZE_TO_STRING_METHOD "serializeToString"

#define PB_FIELD_NAME "name"
#define PB_FIELD_REQUIRED "required"
#define PB_FIELD_TYPE "type"
#define PB_VALUES_PROPERTY "values"

static void f_printf(const String& name, const Variant& params) {
  f_hphp_invoke(name,params);
}

enum {
    PB_TYPE_DOUBLE = 1,
    PB_TYPE_FIXED32,
    PB_TYPE_FIXED64,
    PB_TYPE_FLOAT,
    PB_TYPE_INT,
    PB_TYPE_SIGNED_INT,
    PB_TYPE_STRING,
    PB_TYPE_BOOL,
    PB_TYPE_UINT64
};

static int64_t PB_FIELD_TYPE_HASH;
static int64_t PB_VALUES_PROPERTY_HASH;

//static funtion declar
static Array pb_get_values(
    const Object& obj);
static Variant pb_get_value(
    const Object& obj, 
    Array& values, 
    int64_t field_number);
static int64_t pb_assign_value(
    const Object& obj, 
    Variant& dst, 
    CVarRef src, 
    int64_t field_number);
static Array pb_get_field_descriptors(
    const Object& obj);
static Array pb_get_field_descriptor(
    const Object& obj, 
    Array& field_descriptors, 
    int64_t field_number);
static Variant pb_get_field_type(
    const Object& obj, 
    Array& field_descriptor, 
    int64_t field_number);
static const char *pb_get_field_name(
    const Object& obj, 
    int64_t field_number);
static int32_t pb_dump_field_value(
    CVarRef value, 
    int64_t level, 
    bool only_set);
static const char *pb_get_wire_type_name(int wire_type);
static int pb_serialize_field_value(
    const Object& obj, 
    writer_t *writer, 
    uint32_t field_number, 
    CVarRef type, 
    CVarRef value);
static int pb_get_wire_type(int field_type);
static void throw_exception(const char* name, int line, const char *format, ...);
static int pb_write_uint64(const Object& obj, writer_t *writer, 
    uint32_t field_number, CVarRef value);

//function 
#define PB_COMPILE_ERROR(message, ...) throw_exception(__FILE__, __LINE__, \
"compile error - " #message, __VA_ARGS__)
#define PB_PARSE_ERROR(message, ...) throw_exception(__FILE__, __LINE__, \
    "parse error - " #message, __VA_ARGS__)


void HHVM_METHOD(ProtobufMessage, append, int64_t position, CVarRef value) {
    //检查参数
    if (value.isNull()) {
        return;
    }
    //获取成员values
    Array values = pb_get_values(Object{this_});
    //从values中获取字段内容，append只适用于repeated字段
    Variant in_value = pb_get_value(Object{this_}, values, position); 
    if (!in_value.isArray()) {
        PB_COMPILE_ERROR("'%s' field internal type should be an array", 
            pb_get_field_name(Object{this_}, position));
        return;
    }
    Array array = in_value.toArray();
    //根据proto文件，矫正参数类型
    Variant dst;
    if (pb_assign_value(Object{this_}, dst, value, position) != 0) {
        return;
    }
    //将矫正后的参数添加到字段内容里
    array.append(dst);
    //将新字段替换到values
    values.set(position, array);
    //将values替换到类变量
    Object{this_}->o_set(PB_VALUES_PROPERTY, values);
}

void HHVM_METHOD(ProtobufMessage, clear, int64_t position) {
    //获取类变量values
    Array values = pb_get_values(Object{this_});
    values.set(position, Array::Create());
    //将values替换到类变量
    Object{this_}->o_set(PB_VALUES_PROPERTY, values);
}

void HHVM_METHOD(ProtobufMessage, dump, 
                   bool onlySet/*=true*/, 
                   int64_t indentation/*=0*/) {    
    Array values = pb_get_values(Object{this_});
    Array field_descriptors = pb_get_field_descriptors(Object{this_});

    if (indentation > 0) {
        String format = "%*c%s {\n";
        Array args;
        args.append(2*(int)indentation);
        args.append(' ');
        args.append(Object{this_}->getVMClass()->name()->data());
        f_printf( format, args);        
    } else {
        String format = "%s {\n";
        Array args;
        args.append(Object{this_}->getVMClass()->name()->data());
        f_printf(format, args);  
    }

    for (ArrayIter i(values); i; ++i) {
        int64_t field_number = i.first().toInt64();
        Variant value = i.second();

        Array field_descriptor = pb_get_field_descriptor(Object{this_}, field_descriptors, field_number);
        if (field_descriptor.isNull()) {
            return;
        }
        Variant field_name = pb_get_field_name(Object{this_}, field_number);
        if (!field_name.isInitialized()) {
            return;
        }

        if (value.isArray()) {
            if (value.toArray().size() > 0 || !onlySet) {
                String format = "%*c%lu: %s(%d) => \n";
                Array args;
                args.append(((int) indentation + 1) * 2);
                args.append(' ');
                args.append(field_number);
                args.append(field_name);
                args.append(value.toArray().size());
                f_printf(format, args);  

                if (value.toArray().size() > 0) {
                    for (ArrayIter j(value); j; ++j) {
                        Variant index = i.first();
                        Variant val = i.second();

                        String format = "%*c[%lu] =>";
                        Array args;
                        args.append(((int) indentation + 2) * 2);
                        args.append(' ');
                        args.append(index);
                        f_printf(format, args);  

                        if (pb_dump_field_value(val, indentation + 3, onlySet) != 0) {
                            return;
                        }
                    }
                } else  {
                    String format = "%*cempty\n";
                    Array args;
                    args.append(((int) indentation + 2) * 2);
                    args.append(' ');
                    f_printf(format, args);  
                }
            }
        } else if (value.isInitialized() || !onlySet) {
            String format = "%*c%lu: %s =>";
            Array args;
            args.append(2 * ((int) indentation + 1));
            args.append(' ');
            args.append(field_number);
            args.append(field_name);
            f_printf(format, args);  

            if (pb_dump_field_value(value, indentation + 1, onlySet) != 0) {
                return;
            }
        }
    }
}

int64_t HHVM_METHOD(ProtobufMessage, count, int64_t position) {
    //获取成员values
    Array values = pb_get_values(Object{this_});
    //从values中获取字段内容
    Variant value = pb_get_value(Object{this_}, values, position);
    //获取value的长度    
    if (value.isArray()) {
        return value.toArray().size();
    } else {
        PB_COMPILE_ERROR("'%s' field internal type should be an array", 
            pb_get_field_name(Object{this_}, position));
        return 0;
    }
}

Variant HHVM_METHOD(ProtobufMessage, get, int64_t position, int64_t offset/*=-1*/) {
    Variant temp;
    //获取成员values
    Array values = pb_get_values(Object{this_});
    //从values中获取字段内容
    temp = pb_get_value(Object{this_}, values, position);    
    
    if (offset != -1) {
        if (!temp.isInitialized() || !temp.isArray()) {
            PB_COMPILE_ERROR("'%s' field internal type should be an array", 
                pb_get_field_name(Object{this_}, position));
            return temp;
        } else {
            return temp.toArray().rvalAt(offset);
        }
    } else {
        return temp;
    }
}

bool HHVM_METHOD(ProtobufMessage, parseFromString, const String& packed) {
    //先reset/init下
    Array method = Array::Create();
    method.append(Object{this_});
    method.append(PB_RESET_METHOD);
    f_call_user_func(method);

    Array field_descriptors = pb_get_field_descriptors(Object{this_});
    Array values = pb_get_values(Object{this_});
    //parse
    reader_t reader;
    reader_init(&reader, (char*)packed.data(), packed.length());

    int64_t field_number = 0;
    uint8_t wire_type = 0;
    int ret = 0;
    while (reader_has_more(&reader)) {
        if (reader_read_tag(&reader, (uint32_t*)&field_number, &wire_type) != 0 ||\
            field_number == 0) {
            break;
        }

        Array field_descriptor = pb_get_field_descriptor(Object{this_}, field_descriptors, field_number);
        if (field_descriptor.isNull()) { 
            switch (wire_type)
            {
            case WIRE_TYPE_VARINT:
                ret = reader_skip_varint(&reader);
                break;

            case WIRE_TYPE_64BIT:
                ret = reader_skip_64bit(&reader);
                break;

            case WIRE_TYPE_LENGTH_DELIMITED:
                ret = reader_skip_length_delimited(&reader);
                break;

            case WIRE_TYPE_32BIT:
                ret = reader_skip_32bit(&reader);
                break;

            default:
                PB_PARSE_ERROR("unexpected wire type %d for unexpected %ld field", 
                    wire_type, field_number);
                return false;
            }

            if (ret != 0) {
                PB_PARSE_ERROR("parse unexpected %ld field of wire type %s fail", 
                    field_number, pb_get_wire_type_name(wire_type));
                return false;
            }

            continue;
        }

        Variant field_type = pb_get_field_type(Object{this_}, field_descriptor, field_number);
        if (field_type.isNull()) {
            return false;
        }

        Variant value = pb_get_value(Object{this_}, values, field_number);
        if (!value.isInitialized()) {
            return false;
        }

        if (field_type.isString()) {
            if (wire_type != WIRE_TYPE_LENGTH_DELIMITED) {
                PB_PARSE_ERROR("'%s' field wire type is %s but should be %s", 
                    pb_get_field_name(Object{this_}, field_number), 
                    pb_get_wire_type_name(wire_type), 
                    pb_get_wire_type_name(WIRE_TYPE_LENGTH_DELIMITED));
                return false;
            }
            if (!f_class_exists(field_type.toString())) {
                PB_COMPILE_ERROR("class %s for '%s' does not exist", 
                    field_type.toString().c_str(), pb_get_field_name(Object{this_}, field_number));
                return false;
            }

            char* subpack = NULL;
            int subpack_size = 0;
            if ((ret = reader_read_string(&reader, &subpack, &subpack_size)) == 0) {
                Array construct = Array::Create();
                Object sub_value = f_hphp_create_object(field_type.toString(),construct);
               
                Array method = Array::Create();
                method.append(sub_value);
                method.append(PB_PARSE_FROM_STRING_METHOD);
                Array params = Array::Create();
                params.append(String(subpack, subpack_size, CopyString));
                Variant ret = f_call_user_func( method, params);
                if (!ret.isBoolean() || !ret.toBoolean()) {
                    return false;
                } else {
                    if (value.isArray()) {
                        value.toArrRef().append(sub_value);                    
                    } else {
                        value = sub_value;
                    }
                    values.set(field_number, value);
                }
            } else {
                PB_PARSE_ERROR("parse '%s' field fail", pb_get_field_name(Object{this_}, field_number));
                return false;
            }
        } else if (field_type.isInteger()) {
            if (pb_get_wire_type(field_type.toInt64()) != wire_type) {
                PB_PARSE_ERROR("'%s' field wire type is %s but should be %s", 
                    pb_get_field_name(Object{this_}, field_number), 
                    pb_get_wire_type_name(wire_type), 
                    pb_get_wire_type_name(field_type.toByte()));
                return false;
            }

            Variant tmp;
            switch (field_type.toByte())
            {
            case PB_TYPE_DOUBLE:
                {
                    double out = 0.0;
                    ret = reader_read_double(&reader, &out);
                    tmp = out;
                }
                break;

            case PB_TYPE_FIXED32:
                {
                    int64_t out = 0;
                    ret = reader_read_fixed32(&reader, &out);
                    tmp = out;
                }
                break;

            case PB_TYPE_FIXED64:
                {
                    int64_t out = 0;
                    ret = reader_read_fixed64(&reader, &out);
                    tmp = out;
                }
                break;

            case PB_TYPE_FLOAT:
                {
                    double out = 0.0;
                    ret = reader_read_float(&reader, &out);
                    tmp = out;
                }
                break;

            case PB_TYPE_INT:
                {
                    int64_t out = 0;
                    ret = reader_read_int(&reader, &out);
                    tmp = out;
                }
                break;

            case PB_TYPE_BOOL:
                {
                    int64_t out = 0;
                    ret = reader_read_int(&reader, &out);
                    tmp = out;
                    tmp = tmp.toBoolean();
                }
                break;

            case PB_TYPE_SIGNED_INT:
                {
                    int64_t out = 0;
                    ret = reader_read_signed_int(&reader, &out);
                    tmp = out;
                }
                break;

            case PB_TYPE_STRING:
                {
                    char* str = NULL;
                    int str_size = 0;
                    if ((ret = reader_read_string(&reader, &str, &str_size)) == 0) {
                        tmp = String(str, str_size, CopyString);
                    }
                }
                break;

            case PB_TYPE_UINT64:
                {
                    uint64_t out = 0;
                    reader_read_uint64(&reader, &out);
                    std::ostringstream num;
                    num << out;
                    tmp = String(num.str().c_str(), num.str().length(), CopyString);
                }
                break;

            default:
                PB_COMPILE_ERROR("unexpected '%s' field type %d in field descriptor", 
                    pb_get_field_name(Object{this_}, field_number), 
                    field_type.toByte());
                return false;
            }

            if (ret != 0) {
                PB_PARSE_ERROR("parse '%s' field fail", pb_get_field_name(Object{this_}, field_number));
                return false;
            } else {
                if (value.isArray()) {
                    value.toArrRef().append(tmp);
                    values.set(field_number, value);
                } else {
                    values.set(field_number, tmp);
                }         
            }
        } else {
            PB_COMPILE_ERROR("unexpected %d type of '%s' field type in field descriptor", 
                field_type.toByte(), pb_get_field_name(Object{this_}, field_number));
            return false;
        }
    }

    Object{this_}->o_set(PB_VALUES_PROPERTY, values);
    return true;
}

String HHVM_METHOD(ProtobufMessage, serializeToString) {
#define SERIALIZE_FAIL_RETURN {\
    writer_free_pack(&writer);\
    return null_string;\
}
    Array field_descriptors = pb_get_field_descriptors(Object{this_});
    Array values = pb_get_values(Object{this_});

    writer_t writer;
    writer_init(&writer);
    
    Variant value;
    for (ArrayIter i(field_descriptors); i; ++i) {
        int64_t field_number = i.first().toInt64();
        Array field_descriptor = i.second().toArray();

        value = values.rvalAt(field_number);
        if (!value.isInitialized()) {
            Variant required = field_descriptor.rvalAt(String(PB_FIELD_REQUIRED));
            if (!required.isInitialized()) {
                PB_COMPILE_ERROR("missing '%s' field required property in field descriptor", 
                    pb_get_field_name(Object{this_}, 
                    field_number));
                SERIALIZE_FAIL_RETURN
            }

            if (required.toBoolean()) {
                PB_COMPILE_ERROR("this '%s' field is required and must be set", 
                    pb_get_field_name(Object{this_}, field_number));
                SERIALIZE_FAIL_RETURN
            }

            continue;
        }

        Variant type = pb_get_field_type(Object{this_}, field_descriptor, field_number);
        if (!type.isInitialized()) {
            SERIALIZE_FAIL_RETURN
        }

        if (value.isArray()) {
            Array array  = value.toArray();
            for (ArrayIter j(array); j; ++j) {
                Variant sub_value = j.second();
                if (pb_serialize_field_value(Object{this_}, &writer, field_number, type, sub_value) != 0) {
                    SERIALIZE_FAIL_RETURN
                }
            }
        } else if (pb_serialize_field_value(Object{this_}, &writer, field_number, type, value) != 0) {
            SERIALIZE_FAIL_RETURN
        }
    }

    char *pack = NULL;
    int pack_size = 0;
    pack = writer_get_pack(&writer, &pack_size);
    String rel(pack, pack_size, CopyString);
    writer_free_pack(&writer);
    return rel;
}

void HHVM_METHOD(ProtobufMessage, set, int64_t position, CVarRef value) {
    //get member values 
    Array values = pb_get_values(Object{this_});
    if (!values.exists(position)) {
        return;
    }
    values.set(position, value);
    //replace the value with a class variable
    Object{this_}->o_set(PB_VALUES_PROPERTY, values);
}

static class ProtobufExtension : public Extension {
public:
    ProtobufExtension() : Extension("protobuf") {}
    virtual void moduleInit() {
        HHVM_ME(ProtobufMessage, append);
        HHVM_ME(ProtobufMessage, clear);
        HHVM_ME(ProtobufMessage, dump);
        HHVM_ME(ProtobufMessage, count);
        HHVM_ME(ProtobufMessage, get);
        HHVM_ME(ProtobufMessage, parseFromString);
        HHVM_ME(ProtobufMessage, serializeToString);
        HHVM_ME(ProtobufMessage, set);

        loadSystemlib();
    }
} s_protobuf_extension;

#ifndef MARK_AS_UNITTEST
HHVM_GET_MODULE(protobuf)
#endif

static Array pb_get_values(const Object& obj) {
    Variant values = obj.o_get(PB_VALUES_PROPERTY);
    return values.toArray();
}

static Variant pb_get_value(const Object& obj, Array& values, int64_t field_number) {
    Variant value;
    if (values.isNull() ||\
        !(value = values.rvalAt(field_number)).isInitialized()) {
            PB_COMPILE_ERROR("missing %ld field value", field_number);
    }
    return value;
}

static int64_t pb_assign_value(const Object& obj, Variant& dst, 
                               CVarRef src, int64_t field_number) {
#define ASSIGN_FAIL_RETURN return -1
    Array field_descriptors = pb_get_field_descriptors(obj);
    if (field_descriptors.isNull()) {
        ASSIGN_FAIL_RETURN;
    }

    Array field_descriptor = pb_get_field_descriptor(obj, field_descriptors, field_number);
    if (field_descriptor.isNull()) {
        ASSIGN_FAIL_RETURN;
    }

    Variant type = pb_get_field_type(obj, field_descriptor, field_number);
    if (!type.isInitialized()) {
        ASSIGN_FAIL_RETURN;
    }

    Variant tmp = src;
    if (type.isInteger()) {
        switch (type.toInt64())
        {
        case PB_TYPE_DOUBLE:
        case PB_TYPE_FLOAT:
            if (!tmp.isDouble()) {
                tmp = tmp.toDouble();
            }
            break;

        case PB_TYPE_FIXED32:
        case PB_TYPE_INT:
        case PB_TYPE_FIXED64:
        case PB_TYPE_SIGNED_INT:
        case PB_TYPE_BOOL:
            if (!tmp.isInteger()) {
                tmp = tmp.toInt64();
            }
            break;

        case PB_TYPE_STRING:
        case PB_TYPE_UINT64:
            if (!tmp.isString()) {
                tmp = tmp.toString();
            }
            break;

        default:
            PB_COMPILE_ERROR("unexpected '%s' field type %d in field descriptor", 
                pb_get_field_name(obj, field_number), type.toInt32());
            ASSIGN_FAIL_RETURN;
        }
    } else if (!type.isString()) {
        ASSIGN_FAIL_RETURN;
    }

    dst = tmp;
    return 0;
}

static Array pb_get_field_descriptors(const Object& obj) {
    Array method = Array::Create();
    method.append(obj);
    method.append(PB_FIELDS_METHOD);

    Variant descriptors = f_call_user_func(method);
    Array array;
    if (!descriptors.isArray() ||\
        (array = descriptors.toArray()).isNull()) {
        PB_COMPILE_ERROR("missing fields descriptor%d", 0);
    }
    return array;
}

static Array pb_get_field_descriptor(const Object& obj, 
                                       Array& field_descriptors, int64_t field_number) {
    Variant field_descriptor;    
    if (field_descriptors.isNull() ||\
        !(field_descriptor = field_descriptors.rvalAt(field_number)).isInitialized()) {
            PB_COMPILE_ERROR("missing %ld field descriptor", field_number);
    } else if (field_descriptor.isArray()) {
        return field_descriptor.toArray();
    }

    return Array();
}

static Variant pb_get_field_type(const Object& obj, 
                                 Array& field_descriptor, int64_t field_number) {
    Variant field_type = field_descriptor.rvalAt(String(PB_FIELD_TYPE));
    if (!field_type.isInitialized()) {
        PB_COMPILE_ERROR("missing '%s' field type property in field descriptor", 
            pb_get_field_name(obj, field_number));
    }
    return field_type;
}

static const char *pb_get_field_name(const Object& obj, int64_t field_number) {
    Array field_descriptors = pb_get_field_descriptors(obj);
    if (field_descriptors.isNull()) {
        return NULL;
    }

    Array field_descriptor = pb_get_field_descriptor(obj, field_descriptors, field_number);
    if (field_descriptor.isNull()) {
        PB_COMPILE_ERROR("missing '%s' field type property in field descriptor", 
            pb_get_field_name(obj, field_number));
        return NULL;
    }

    Variant field_name = field_descriptor.rvalAt(String(PB_FIELD_NAME));
    if (!field_name.isInitialized()) {
        PB_COMPILE_ERROR("missing %ld field name property in field descriptor", 
            field_number);
        return NULL;
    }

    return field_name.toString().c_str();
}

static int32_t pb_dump_field_value(CVarRef value, int64_t level, bool only_set) {
    String content;
    if (value.isObject()) {
        String format = "\n";
        f_printf(0, format);  

        Array method = Array::Create();
        method.append(value);
        method.append(PB_DUMP_METHOD);
        Array params = Array::Create();
        params.append(only_set);
        params.append(level);

        f_call_user_func( method, params);
    } else if (value.isNull()) {
        content = "null (not set)";
    } else if (value.isBoolean()) {
        if (value.toBoolean()) {
            content = "true";
        } else {
            content  ="false";
        }
    } else {
        content = value.toString();
    }

    if (value.isString()) {
        String format = " '%s'\n";
        Array args;
        args.append(content);
        f_printf(format, args);  
    } else {
        String format = " %s\n";
        Array args;
        args.append(content);
        f_printf(format, args); 
    }

    return 0;
}

static const char *pb_get_wire_type_name(int wire_type) {
    const char *name;

    switch (wire_type)
    {
    case WIRE_TYPE_VARINT:
        name = "varint";
        break;

    case WIRE_TYPE_64BIT:
        name = "64bit";
        break;

    case WIRE_TYPE_LENGTH_DELIMITED:
        name = "length-delimited";
        break;

    case WIRE_TYPE_32BIT:
        name = "32bit";
        break;

    default:
        name = "unknown";
        break;
    }

    return name;
}

static int pb_serialize_field_value(const Object& obj, writer_t *writer, uint32_t field_number, 
    CVarRef type, CVarRef value) {
    if (value.isNull()) {
        return 0;
    }
    if (type.isString()) {
        Array method = Array::Create();
        method.append(value);   //sub class obj
        method.append(PB_SERIALIZE_TO_STRING_METHOD);
        Variant ret = f_call_user_func( method);
    
        if (!ret.isString()) {
            return -1;
        } else {
            String sub = ret.toString();
            if (sub.length() > 0) {
                writer_write_message(writer, field_number, 
                    sub.c_str(), sub.length());
            }
        }
    } else if (type.isInteger()) {
        int r = 0;
        switch (type.toInt64())
        {
        case PB_TYPE_DOUBLE:
            r = writer_write_double(writer, field_number, value.toDouble());
            break;
    
        case PB_TYPE_FIXED32:
            r = writer_write_fixed32(writer, field_number, value.toInt64());
            break;
    
        case PB_TYPE_INT:
        case PB_TYPE_BOOL:
            r = writer_write_int(writer, field_number, value.toInt64());
            break;
    
        case PB_TYPE_FIXED64:
            r = writer_write_fixed64(writer, field_number, value.toInt64());
            break;
    
        case PB_TYPE_FLOAT:
            r = writer_write_float(writer, field_number, value.toDouble());
            break;
    
        case PB_TYPE_SIGNED_INT:
            r = writer_write_signed_int(writer, field_number, value.toInt64());
            break;
    
        case PB_TYPE_STRING:
            r = writer_write_string(writer, field_number, 
                    value.toString().c_str(), value.toString().length());
            break;

        case PB_TYPE_UINT64:
            r = pb_write_uint64(obj, writer, field_number, value);
            break;

        default:
            PB_COMPILE_ERROR("unexpected '%s' field type %ld in field descriptor", 
                pb_get_field_name(obj, field_number), type.toInt64());
            return -1;
        }
    
        if (r != 0) {
            PB_COMPILE_ERROR("func(writer_write_string) return:%d", r);
            return -1;
        }
    } else {
        PB_COMPILE_ERROR("unexpected type%d", 0);
        return -1;
    }
    
    return 0;
}

static int pb_get_wire_type(int field_type) {
    int ret = -1;

    switch (field_type)
    {
        case PB_TYPE_DOUBLE:
        case PB_TYPE_FIXED64:
            ret = WIRE_TYPE_64BIT;
            break;

        case PB_TYPE_FIXED32:
        case PB_TYPE_FLOAT:
            ret = WIRE_TYPE_32BIT;
            break;

        case PB_TYPE_INT:
        case PB_TYPE_SIGNED_INT:
        case PB_TYPE_BOOL:
        case PB_TYPE_UINT64:
            ret = WIRE_TYPE_VARINT;
            break;

        case PB_TYPE_STRING:
            ret = WIRE_TYPE_LENGTH_DELIMITED;
            break;

        default:
            ret = -1;
            break;
    }

    return ret;
}

static int pb_write_uint64(const Object& obj, writer_t *writer, 
                           uint32_t field_number, CVarRef value) {
    if (!value.isString()) {
        PB_PARSE_ERROR("'%s' field must is string, now type is %d", 
            pb_get_field_name(obj, field_number),
            value.getType());
        return -1;
    }

    uint64_t num = strtoull(value.toString().c_str(), NULL, 10);
    return writer_write_uint64(writer, field_number, num);
}

static void throw_exception(const char* name, int line, const char *format, ...) {
    va_list args;
    va_start(args, format);
    char buffer[1024];
    vsnprintf(buffer, sizeof(buffer), format, args);
    char out[1024];
    snprintf(out, sizeof(out), "[%s : %d] %s", name, line, buffer);
    va_end(args);
    throw Object(SystemLib::AllocExceptionObject(out));
}

} /*hphp*/
