#ifndef __class_demoI_h__
#define __class_demoI_h__

#include <class_demo.h>
#include <iostream>
using namespace std;

namespace com
{

namespace jd
{

class CommontIceRpcServiceI : virtual public CommontIceRpcService
{
public:

    virtual ::com::jd::ClassDemoPtr getClassDemo(const Ice::Current&);

    virtual ::com::jd::ClassDemoPtr newClassDemo(const ::com::jd::ClassDemoPtr&,
                                                 const Ice::Current&);

    virtual ::com::jd::ClassDemoS getClassDemoS(const Ice::Current&);

    virtual void setClassDemos(const ::com::jd::ClassDemoS&,
                               const Ice::Current&);

    virtual ::com::jd::LongList getLongList(const Ice::Current&);

    virtual ::com::jd::LongList setLongList(const ::com::jd::LongList&,
                                            const Ice::Current&);
	virtual ::Ice::Int getOtherCommentById(const Ice::Current&);


										
};

}

}

#endif
