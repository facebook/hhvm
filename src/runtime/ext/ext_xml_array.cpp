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
    ///////////////////////////////////////////////////////////////////////////////
    /**
    class eArrayVal {
        public:
        int      layer ;
        Array    *arr;

        eArrayVal(Array * array, int layerP) {
            this->arr    = array;
            this->layer  = layerP;
        }
    };
    */
    struct eArrayVal
    {
        eArrayVal(Array * z, int l) :
            arr(z), layer(l)
        {
        }

        Array *	        arr;
        int		layer ;
    };

    class XmlArrResult {
    public:
        Array result;
        std::stack<eArrayVal> astack;
        XmlArrResult(){
            //result  = Array::Create();
            astack.push(eArrayVal(&result, 1));
        }

        ~XmlArrResult() {
            while(astack.size() > 0) {
                astack.pop();
            }
        }
    };

    #define XML_STACK_TOP_ARR_DATA ((XmlArrResult *)(data))->astack.top().arr
    #define XML_ARR_STACK_POP ((XmlArrResult *)(data))->astack.pop()
    #define XML_ARR_STACK_TOP ((XmlArrResult *)(data))->astack.top()

    Array xml_proc(CStrRef xmlStr);
    int is_multibyte(const char *s, int len);
    void xml_cdata(void * ctx, const xmlChar *cdata , int len);
    void xml_start(void *data, const xmlChar *xmlName, const xmlChar **attr);
    void ignorableWhitespace(void * ctx, const xmlChar * ch, int len);
    static void xml_end(void *data, const xmlChar *el);
    static void xml_text(void * ctx, const xmlChar *c, int len);
    static void xml_attr(void * data, const xmlChar **attr);


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

        return xml_proc(xml);
    }

    static bool is_blank(const xmlChar* str) {
        while (*str != '\0') {
            if (*str != ' '  && *str != 0x9 && *str != 0xa && *str != 0xd) {
                return false;
            }
            str++;
        }
        return true;
    }
    static void cleanup_xml_node(xmlNodePtr node) {
        xmlNodePtr trav;
        xmlNodePtr del = NULL;

        trav = node->children;
        while (trav != NULL) {
            if (del != NULL) {
                xmlUnlinkNode(del);
                xmlFreeNode(del);
                del = NULL;
            }
            if (trav->type == XML_TEXT_NODE) {
                if (is_blank(trav->content)) {
                    del = trav;
                }
            } else if ((trav->type != XML_ELEMENT_NODE) &&
                       (trav->type != XML_CDATA_SECTION_NODE)) {
                del = trav;
            } else if (trav->children != NULL) {
                cleanup_xml_node(trav);
            }
            trav = trav->next;
        }
        if (del != NULL) {
            xmlUnlinkNode(del);
            xmlFreeNode(del);
        }
    }

    Array xml_proc(CStrRef xmlStr) {
        XmlArrResult *xmlResult = new XmlArrResult;
        xmlSubstituteEntitiesDefault(1);
        xmlParserCtxtPtr ctxt = NULL;
        xmlDocPtr ret;
        /**
        xmlInitParser();
        **/

        ctxt = xmlCreateMemoryParserCtxt(xmlStr.data(), xmlStr.size());
        if (ctxt) {
            xmlSAXHandlerPtr saxHandlerPtr = (xmlSAXHandlerPtr)calloc(1, sizeof(xmlSAXHandler));
            saxHandlerPtr->cdataBlock = xml_cdata;
            saxHandlerPtr->startElement= xml_start;
            saxHandlerPtr->endElement= xml_end;
            saxHandlerPtr->ignorableWhitespace = ignorableWhitespace;
            saxHandlerPtr->characters =  xml_text;
            ctxt->sax = saxHandlerPtr;
            ctxt->disableSAX = 0;
            ctxt->keepBlanks = 0;
            ctxt->userData = (void *)xmlResult;
            ctxt->sax->warning = NULL;
            ctxt->sax->error = NULL;
            /*ctxt->sax->fatalError = NULL;*/
            xmlParseDocument(ctxt);
            if (ctxt->wellFormed) {
                ret = ctxt->myDoc;
                if (ret && ret->URL == NULL && ctxt->directory != NULL) {
                    ret->URL = xmlCharStrdup(ctxt->directory);
                }
            } else {
                ret = NULL;
                xmlFreeDoc(ctxt->myDoc);
                ctxt->myDoc = NULL;
            }
            xmlFree(saxHandlerPtr);
            ctxt->sax = NULL;
            xmlFreeParserCtxt(ctxt);
            ctxt = NULL;
        } else {
            ret = NULL;
        }

        /*
           xmlCleanupParser();
        */
        if (ret) {
            cleanup_xml_node((xmlNodePtr)ret);
        }


        Array ret_arr(xmlResult->result);

        delete xmlResult;

        return ret_arr;
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

    void xml_cdata(void * data, const xmlChar *cdata , int len) {
        String v_name("_v");
        String c_string ((const char *)cdata, len, CopyString);

        if(is_multibyte((char *)c_string.data(), c_string.size())) {
            c_string = f_iconv("UTF-8","GBK//IGNORE", c_string.data());
        }

        XML_STACK_TOP_ARR_DATA->set(v_name, c_string) ;
        return;
    }

    void xml_start(void *data, const xmlChar *xmlName, const xmlChar **attr)
    {

        char *name = (char *)xmlName;
        String name_str(name,strlen(name),CopyString);

        if(!XML_STACK_TOP_ARR_DATA->exists(name_str)){
                Array new_array(Array::Create());
                XML_STACK_TOP_ARR_DATA->set(name_str, new_array);
                Array &arr_input(XML_STACK_TOP_ARR_DATA->lvalAt(name_str).toArrRef());
                ((XmlArrResult *)(data))->astack.push(eArrayVal((&arr_input),1));
		xml_attr(data, attr);
		return ;
	}

        if(!(*XML_STACK_TOP_ARR_DATA)[name_str].is(KindOfArray)) {
            raise_warning("sorry, xml_array critical error!");
            return;
        }

        Array &zvalue_arr(XML_STACK_TOP_ARR_DATA->lvalAt(name_str).toArrRef());
        int64 zero = 0l;
        if(!zvalue_arr.exists(zero)) {
		Array old_array = (*XML_STACK_TOP_ARR_DATA)[name_str].toArray();
                Array new_array(Array::Create());
                Array one_array(Array::Create());
                new_array.set(zero, old_array);
                new_array.set(zero+1, one_array);
                XML_STACK_TOP_ARR_DATA->set(name_str, new_array);
                Array &new_array_ref(XML_STACK_TOP_ARR_DATA->lvalAt(name_str).toArrRef());
                Array &top_array_ref(new_array_ref.lvalAt(1).toArrRef());

                ((XmlArrResult *)(data))->astack.push(eArrayVal((&new_array_ref),1));
                ((XmlArrResult *)(data))->astack.push(eArrayVal(&top_array_ref,2));
                xml_attr(data, attr);
	} else {
                Array &zvalue_arr(XML_STACK_TOP_ARR_DATA->lvalAt(name_str).toArrRef());
                ((XmlArrResult *)(data))->astack.push(eArrayVal((&zvalue_arr),1));
                int top_array_size = zvalue_arr->size();
                Array one_array (Array::Create());
                XML_STACK_TOP_ARR_DATA->set(top_array_size,one_array);
                Array &top_array_ref(XML_STACK_TOP_ARR_DATA->lvalAt(top_array_size).toArrRef());
                ((XmlArrResult *)(data))->astack.push(eArrayVal(&top_array_ref,2));
                xml_attr(data, attr);
	}
	return ;
    }


    static void xml_end(void *data, const xmlChar *el) {
        int pop_loop = XML_ARR_STACK_TOP.layer;
        for ( int i = 0 ; i < pop_loop ; i++ ) {
            XML_ARR_STACK_POP;
        }
        return ;
    }

    void ignorableWhitespace(void * ctx, const xmlChar * ch, int len) {

    }

    //貌似很简单
    static void xml_text(void * data, const xmlChar *c, int len)
    {
        String v_name("_v");
        String s_string((const char *)c,len, CopyString);

        if(is_multibyte((char *)c, len)) {
            s_string = f_iconv("UTF-8","GBK//IGNORE", s_string.data());
        }

	if ( !XML_STACK_TOP_ARR_DATA->exists(v_name)) {
            XML_STACK_TOP_ARR_DATA->set(v_name, s_string);
	} else if ( (*XML_STACK_TOP_ARR_DATA)[v_name].is(KindOfString)) {
            s_string = (*XML_STACK_TOP_ARR_DATA)[v_name].toString() + s_string;
            XML_STACK_TOP_ARR_DATA->set(v_name, s_string);
        } else {
            raise_warning("xml_array: invalid text");
	}
        return ;
    }

    static void xml_attr(void *data, const xmlChar **attr) {
        /**
        //SIMPLE IT
	char *key, *value;
        if (attr != NULL) {
            Array attrs = Array::Create();
            while ( (key = (char*)*attr++) && (value = (char*)*attr++) ) {
                String c_string = String(value);
                if(is_multibyte(value, strlen(value))) {
                    c_string = f_iconv("UTF-8","GBK//IGNORE", c_string);
                }
                attrs.set(String(key), String(value));
            }
            XML_STACK_TOP_ARR_DATA->set(String("_p"), attrs);
        }
        */
	char *key, *value;
        String p_name("_p");
        if (attr != NULL) {
            while ( (key = (char*)*attr++) && (value = (char*)*attr++) ) {
                String value_str(value,strlen(value), CopyString);
                String key_str(key,strlen(key), CopyString);

                if(is_multibyte(value_str.data(), value_str.size())) {
                    value_str = f_iconv("UTF-8","GBK//IGNORE", value_str.data());
                }

                if(!XML_STACK_TOP_ARR_DATA->exists(p_name)) {
                    Array new_array(Array::Create());
                    new_array.set(key_str, String(value_str));
                    XML_STACK_TOP_ARR_DATA->set(p_name, new_array);
                } else {
                    Array &p(XML_STACK_TOP_ARR_DATA->lvalAt(p_name).toArrRef());
                    p.set(key_str, value_str);
                }
            }
        }
	return;
    }
    ///////////////////////////////////////////////////////////////////////////////
}
