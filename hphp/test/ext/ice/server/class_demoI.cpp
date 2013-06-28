
#include <class_demoI.h>

::com::jd::ClassDemoPtr
com::jd::CommontIceRpcServiceI::getClassDemo(const Ice::Current& current)
{
	 IceUtil::ThreadControl::sleep(IceUtil::Time::milliSeconds(100));
	ClassDemoPtr cdp=new ClassDemo;
	cdp->idlen=100;
	cdp->id="test";
	APtr a=new A;
	a->id="aaa";
	a->num=111;
	BPtr b=new B;
	b->id="bbb";
	b->name="hzg";
	cdp->classA=a;
	cdp->classB=b;
	cout<<"call getClassDemo"<<endl;

    return cdp;
}

::com::jd::ClassDemoPtr
com::jd::CommontIceRpcServiceI::newClassDemo(const ::com::jd::ClassDemoPtr& cd,
                                             const Ice::Current& current)
{
	cout<<"call newClassDemo"<<endl;
	cout<<cd->id<<"|"<<cd->idlen<<endl;
	cout<<cd->classA->id<<"|"<<cd->classA->num<<endl;
	cout<<cd->classB->id<<"|"<<cd->classB->name<<endl;
    return cd;
}

::com::jd::ClassDemoS
com::jd::CommontIceRpcServiceI::getClassDemoS(const Ice::Current& current)
{
	::com::jd::ClassDemoS classDemoS;
	for(int i=0;i<5;i++){
			ClassDemoPtr cdp=new ClassDemo;
			cdp->id="aaa";
			cdp->idlen=i;
			APtr a=new A;
			a->id="class A";
			a->num=i+11;
			BPtr b=new B;
			b->id="class B";
			b->name="hzg";
			cdp->classA=a;
			cdp->classB=b;
			classDemoS.push_back(cdp);
	}
	for (std::vector< ::com::jd::ClassDemoPtr >::iterator itr = classDemoS.begin(); itr != classDemoS.end(); ++itr){
			::com::jd::ClassDemoPtr cdp=*itr;
			cout<<cdp->id<<endl;
			cout<<cdp->idlen<<endl;
			cout<<"========================="<<endl;
	}
	cout<<"call getClassDemoS"<<endl;
	return classDemoS;
}
	

void
com::jd::CommontIceRpcServiceI::setClassDemos(const ::com::jd::ClassDemoS& cds,
                                              const Ice::Current& current)
{
	cout<<"call setClassDemos"<<endl;
	::com::jd::ClassDemoS classDemoS=cds;
	for (std::vector< ::com::jd::ClassDemoPtr >::iterator itr = classDemoS.begin(); itr != classDemoS.end(); ++itr){
			::com::jd::ClassDemoPtr cdp=*itr;
			cout<<cdp->id<<"|"<<cdp->idlen<<endl;
			cout<<cdp->classA->id<<"|"<<cdp->classA->num<<endl;
			cout<<cdp->classB->id<<"|"<<cdp->classB->name<<endl;

	}

}

::com::jd::LongList
com::jd::CommontIceRpcServiceI::getLongList(const Ice::Current& current)
{
	::com::jd::LongList longList;
	longList.push_back(1);
	longList.push_back(2);
	longList.push_back(3);
	longList.push_back(4);
	cout<<"getLongList"<<endl;
    return longList;
}

::com::jd::LongList
com::jd::CommontIceRpcServiceI::setLongList(const ::com::jd::LongList& ll,
                                            const Ice::Current& current)
{
		cout<<"setLongList"<<endl;
		::com::jd::LongList longList=ll;
		//for (std::vector< ::Ice::Long>::iterator itr = ll.begin(); itr != ll.end(); ++itr){

		for (std::vector< ::Ice::Long>::iterator itr = longList.begin(); itr != longList.end(); ++itr){
				cout<<*itr<<"============"<<endl;
		}

    return ll;
}

 ::Ice::Int com::jd::CommontIceRpcServiceI::getOtherCommentById(const Ice::Current&){
    ::com::jd::RpcException re;
	re.resultCode=500;
	re.msg="problem error!";
	throw re;
	return 5555;
 }

