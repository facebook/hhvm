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

#include "ext_xml_array.h"
#include <runtime/ext/ext_string.h>
#include <runtime/ext/ext_preg.h>
#include <runtime/ext/ext_iconv.h>

#include <libxml/globals.h>
#include <libxml/xmlerror.h>
#include <libxml/parser.h>
#include <libxml/parserInternals.h> /* only for xmlNewInputFromFile() */
#include <libxml/tree.h>
#include <libxml/debugXML.h>
#include <libxml/xmlmemory.h>
#include <runtime/ext/ext_variable.h>
#include <runtime/ext/ext_array.h>

namespace HPHP {
    int i = 0;
    Array *b;
    ///////////////////////////////////////////////////////////////////////////////
    /**
    struct eArrayVal
    {
        eArrayVal(Array z, int l) :
            arr(z), layer(l)
        {
        }
        Array arr;
        int      layer ;
    };
    */
    class eArrayVal {
        public:
        int      layer ;
        Array    *arr;

        eArrayVal(Array * array, int layerP) {
            this->arr    = array;
            this->layer  = layerP;
        }
    };


    IMPLEMENT_THREAD_LOCAL(std::stack<eArrayVal>, estack);
    IMPLEMENT_THREAD_LOCAL(std::vector<Array>, eArrayStack);

    #define XML_ARR_STACK (*estack.get())
    #define XML_ARR_STACK_PTR estack.get()
    #define XML_ARR_STACK_TOP (*estack.get()).top()
    #define XML_ARR_STACK_TOP_ARR ((*estack.get()).top().arr)
    #define XML_ARR_STACK_TOP_ARR_PTR ((*estack.get()).top().arr)
    #define XML_ARR_STACK_PUSH(arr, level) (*estack.get()).push(eArrayVal(arr,level))
    #define XML_ARR_STACK_POP (*estack.get()).pop()


    void xml_proc(CStrRef xmlStr);
    int is_multibyte(const char *s, int len);
    void xml_cdata(void * ctx, const xmlChar *cdata , int len);
    void xml_start(void *data, const xmlChar *xmlName, const xmlChar **attr);
    void ignorableWhitespace(void * ctx, const xmlChar * ch, int len);
    static void xml_end(void *data, const xmlChar *el);
    static void xml_text(void *userData, const xmlChar *c, int len);
    static void xml_attr(VRefParam val, const xmlChar **attr);

    Array * create_array() {
        (*(eArrayStack.get())).push_back(Array::Create());
        Array *ret = &(*(*(eArrayStack.get())).end());
        return ret;
    }

    Variant f_xml_array(CStrRef xmlStr) {
        //fix xmlString start with space
        String xmlTrimed = f_trim(xmlStr);
        //fix illg
        String xml = f_preg_replace("/[\\x00-\\x08\\x0b-\\x0c\\x0e-\\x1f]/", "", xmlTrimed);
        //empty raise waning
        if(xml.size() == 0) {
            raise_warning("empty xml string not start with < ");
            return null_variant;
        }

        //raise xml error before pars
        if(same(f_strpos(xml, "<"), false)) {
            raise_warning("xml not start with < ");
            return null_variant;
        }

        Array *ret = create_array();
        XML_ARR_STACK_PUSH(ret, 1) ;
        xml_proc(xml);
        return ret;
    }

    void xml_proc(CStrRef xmlStr) {

        xmlSAXHandlerPtr saxHandlerPtr = (xmlSAXHandlerPtr)calloc(1, sizeof(xmlSAXHandler));

        saxHandlerPtr->cdataBlock = xml_cdata;
        saxHandlerPtr->startElement= xml_start;
        saxHandlerPtr->endElement= xml_end;
        saxHandlerPtr->ignorableWhitespace = ignorableWhitespace;
        saxHandlerPtr->characters =  xml_text;

        //fix attribute ampersand auto convert to  &#38;
        int oldSubstitute = xmlSubstituteEntitiesDefault(1);

        xmlSAXUserParseMemory(saxHandlerPtr, NULL, xmlStr.data(), xmlStr.size());
        xmlErrorPtr err = xmlGetLastError();
        if(err != NULL) {
            raise_warning("file: %s,  error: %d  %d, message: %s",err->file, err->code,err->level, err->message);
        }

        free(saxHandlerPtr);
        xmlCleanupParser();

        oldSubstitute = xmlSubstituteEntitiesDefault(oldSubstitute);
    }

    /**
     * utf8 单字节首位为0
     * 多字节第一个首位为1  以后每多一个1 表示 多一个字节表示,
     * 就这里 就是用 0x80 & (1000 0000) 判断  多字节
     * 需要传长度 cpp里面解析的时候不会把 字符串截断 在拷贝一份给你的
     */
    int is_multibyte(const char *s, int len) {
        int i = 0;
        for(i=0; i< len; i++){
            if(s[i] & 0x80) {
                return 1;
            }
        }
        return 0;
    }

    void xml_cdata(void * ctx, const xmlChar *cdata , int len) {
        char *c = (char *)cdata;
        String v_name("_v");
        String c_string (c, len, CopyString);

        if(is_multibyte((char *)cdata, len)) {
            c_string = f_iconv("UTF-8","GBK//IGNORE", c_string);
        }
        std::cout << (long long int)XML_ARR_STACK_TOP_ARR_PTR;
        XML_ARR_STACK_TOP_ARR->set(String(v_name), String(c_string.data())) ;
        return;
    }

    void xml_start(void *data, const xmlChar *xmlName, const xmlChar **attr)
    {

        char *name = (char *)xmlName;
        String name_str(name,strlen(name),CopyString);
        Array *new_array = create_array();
        f_var_dump((*(*(eArrayStack.get())).end()));
        if(!XML_ARR_STACK_TOP_ARR->exists(String(name_str.data()))){
            XML_ARR_STACK_TOP_ARR->set(name_str, *new_array);
            //XML_ARR_STACK_PUSH(new_array, 1);
            xml_attr(*new_array, attr);
            return;
        }

        Variant zvalue = (*XML_ARR_STACK_TOP_ARR)[name_str];
        Array zvalue_arr = zvalue.toArray();
        if(!zvalue.is(KindOfArray)) {
            raise_warning("sorry, xml_array critical error!");
            return;
        }
        //XML_ARR_STACK_PUSH(&zvalue_arr, 1);

        if(!zvalue_arr.exists(0)) {
         //   XML_ARR_STACK_TOP_ARR->set(name_str, new_array);

            Array *one_array = create_array();
            new_array->set(0,zvalue_arr);
            new_array->set(1,*one_array);
            XML_ARR_STACK_PUSH(one_array,2);
            xml_attr(one_array, attr);
        } else {
            XML_ARR_STACK_TOP_ARR->append(*new_array);
           // XML_ARR_STACK_PUSH(new_array,2);
            xml_attr(*new_array, attr);
        }
        return;
    }


    static void xml_end(void *data, const xmlChar *el) {
        for ( int i = 0 ; i < XML_ARR_STACK_TOP.layer ; i++ ) {
            XML_ARR_STACK_POP;
        }
        return ;
    }

    void ignorableWhitespace(void * ctx, const xmlChar * ch, int len) {

    }

    //貌似很简单
    static void xml_text(void *userData, const xmlChar *c, int len)
    {
        const char *s= (const char *)c;
        String v_name("_v");
        String s_string(s,len, CopyString);


        if(is_multibyte((char *)c, len)) {
            s_string = f_iconv("UTF-8","GBK//IGNORE", s_string);
        }
        //leak
        //XML_ARR_STACK_TOP_ARR->set(String(v_name.data()), String(s_string.data()));
        return ;
    }

    static void xml_attr(VRefParam val, const xmlChar **attr) {
	char *key, *value;

        if (attr != NULL) {
            Array *attrs = create_array();
            while ( (key = (char*)*attr++) && (value = (char*)*attr++) ) {
                String c_string = String(value);
                if(is_multibyte(value, strlen(value))) {
                    c_string = f_iconv("UTF-8","GBK//IGNORE", c_string);
                }
                (*attrs).set(String(key), String(value));
            }
            XML_ARR_STACK_TOP_ARR->set(String("_p"), *attrs);
        }
	return;
    }
    ///////////////////////////////////////////////////////////////////////////////
}
