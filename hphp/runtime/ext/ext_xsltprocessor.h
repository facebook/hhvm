
#ifndef incl_EXT_XSLTPROCESSOR_H_
#define incl_EXT_XSLTPROCESSOR_H_

#include <libxslt/xslt.h>
#include <libxslt/xsltInternals.h>
#include <libxslt/extensions.h>
#include <libxslt/variables.h>
#include <libxslt/transform.h>
#include <libxslt/xsltutils.h>

#include "hphp/runtime/base/base-includes.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
// class XSLTProcessor

FORWARD_DECLARE_CLASS(XSLTProcessor);
class c_XSLTProcessor : public ExtObjectData, public Sweepable {
 public:
  DECLARE_CLASS(XSLTProcessor)

  // need to implement
  public: c_XSLTProcessor(Class* cls = c_XSLTProcessor::classof());
  public: ~c_XSLTProcessor();
  public: void t___construct();
  public: String t_getparameter(const String& namespaceURI, const String& localName);
  public: bool t_hasexsltsupport();
  public: void t_importstylesheet(CObjRef stylesheet);
  public: void t_registerphpfunctions(CVarRef funcs);
  public: bool t_removeparameter(const String& namespaceURI, const String& localName);
  public: bool t_setparameter(const String& namespaceURI, const String& localName, const String& value);
  public: bool t_setprofiling(const String& filename);
  public: Variant t_transformtodoc(CObjRef doc);
  public: int64_t t_transformtouri(CObjRef doc, const String& uri);
  public: String t_transformtoxml(CObjRef doc);

  public:
    xsltStylesheetPtr m_stylesheet;
    xmlDocPtr m_doc;
    Array m_params;
    int m_registerPhpFunctions;
    Array m_registered_phpfunctions;
    String m_profile;

  private:
    xmlDocPtr apply_stylesheet();
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_EXT_XSLTPROCESSOR_H_
