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

#include <runtime/ext/ext_ice.h>
#include <runtime/ext/ext_class.h>
using std::cout;
using std::endl;

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
const char * Communicator_persistentObject_name="ice_c";
const char * ClassInfoMapVaribaleName="Hiphop_Ice_t_classMap";
/*
	持久化定义对象的key的名字
*/
const char * Ice_Msg_Map_Name="Ice_Msg_Map";
const char * ClassInfo_Map_Name="ClassInfo_Map";
const char * SequenceInfo_Map_Name="SequenceInfo_Map";
const char * ExceptionInfo_Map_Name="ExceptionInfo_Map";
const char * PrimitiveInfo_Map_Name="PrimitiveInfo_Map";

//ICE
c_ICE::c_ICE(const ObjectStaticCallbacks *cb):ExtObjectData(cb){
}

c_ICE::~c_ICE(){
	//communicatorPtr->destroy();
}

void c_ICE::t___construct(){
  INSTANCE_METHOD_INJECTION_BUILTIN(ICE, ICE::__construct);
  //printf("ice object is ok\n");
}

/*
	首先判断传入名称的proxy是否在该线程内的通讯器对象内已经定义，如果未定义，
	那么创建一个iceproxy，并且存入到Ice_Communicator的proxyObjectMap中，保证
	每一个通讯器一个key的proxy只有一个，可以复用，并且在proxy出现问题时，有容错机制
	可以抛出异常
*/
Object c_ICE::t_stringtoproxy(CStrRef str){
  INSTANCE_METHOD_INJECTION_BUILTIN(ICE, ICE::stringtoproxy);
  //c_Proxy *proxy = new c_Proxy();
  c_Proxy *proxy = NEWOBJ(c_Proxy)();
		std::string stringProxy=str.data();
		Ice_Communicator *ic=getIce_CommunicatorByMap();
		proxy->communicatorPtr=communicatorPtr;
		//delete old class factory
		//proxy->factoryWrapper=factoryWrapper;
		proxy->p_factoryWrapper=p_factoryWrapper;
		proxy->p_cict = p_cict;
		Ice::ObjectPrx op=ic->getProxyByName(stringProxy);	
		if(op==NULL){
				char * tidc=getCurrentTid();
				try{
					op = communicatorPtr->stringToProxy(stringProxy);
					//cout<<"create proxy"<<endl;
				}catch (const Ice::Exception& ex){
						//cerr<<"function stringToProxy error:"<<ex<<endl;
						raise_error("function stringToProxy error:%s:",ex.what());
				}
				ic->proxyObjectMap[stringProxy]=op;
				g_persistentObjects->set(Communicator_persistentObject_name,tidc, ic);
		}
		proxy->objectPrx=op;
		return proxy;
}


Object f_ice_find(CStrRef ice_name /* = null */){
	std::string icename = ice_name.data();
	//printf("ice_find!==============\n");

	return NULL;	
}

bool f_ice_register(CObjRef ice_object /*= null*/, CStrRef ice_name /*= null*/, int64 expires /*= null*/){
	//printf("ice_register!==============\n");
	return true;
}

/*
	（1）初始化c_ICE对象，如果已经创建过，则从Ice_Communicator中读取，如果未创建过，
	将新创建的c_ICE对象封装到Ice_Communicator中
	（2）将定过的class与Ice_Communicator中的factoryWrapperClassSet做对比，如果factoryWrapperClassSet
	存在则不插入，如果不存在将对象在factoryWrapperClassSet中创建，并且添加到Ice的工厂中
*/
Object f_ice_initialize() {
	/////////////////////////////////////////////////////////////////////////////
		char * tidc=getCurrentTid();
		Ice_Communicator *ic=getIce_CommunicatorByMap();
		//c_ICE *ice=new c_ICE();
		c_ICE *ice = NEWOBJ(c_ICE)(); 

		bool flag=false;
		try{
		if(ic==NULL){
				ic=new Ice_Communicator();
				ic->cp=Ice::initialize();
				ic->p_iceObjectFactoryWrapperPtr=new P_IceObjectFactoryWrapper();
				flag=true;
		}
		//new ice hzg 20130603
		ResourceMap classResourceMap=g_persistentObjects->getMap(ClassInfo_Map_Name);
		for(ResourceMap::iterator itr =classResourceMap.begin(); itr != classResourceMap.end() ; ++itr){
			std::set <string> ::iterator classSet=ic->factoryWrapperClassSet.find(itr->first);
			if(classSet==ic->factoryWrapperClassSet.end()){
				ic->factoryWrapperClassSet.insert(itr->first);
				ic->cp->addObjectFactory(ic->p_iceObjectFactoryWrapperPtr,itr->first);
				flag=true;
			}

		}
		}catch (const Ice::Exception& ex){
			//cerr<<"function initialize error:"<<ex<<endl;
			raise_error("function initialize error:%s:",ex.what());
		}
		ice->communicatorPtr=ic->cp;
		ice->p_factoryWrapper=ic->p_iceObjectFactoryWrapperPtr;
		if(flag){
			g_persistentObjects->set(Communicator_persistentObject_name,tidc, ic);
		}
		return ice;
}

//IcePHP_class
c_IcePHP_class::c_IcePHP_class(const ObjectStaticCallbacks *cb):ExtObjectData(cb){
		//printf("create a ICE object \n");
}

c_IcePHP_class::~c_IcePHP_class(){
		classInfoMap.clear();
		//printf("c_IcePHP_class destroy \n");
}

/*
	已废弃
*/
c_IcePHP_class* get_hiphopice_classmap(){
		Array dvarname = f_get_defined_vars();
		Variant iceclassmap = dvarname["Hiphop_Ice_t_classMap"];
		if(iceclassmap.isNull()){
			raise_error("Hiphop_Ice_t_classMap variable is NULL or destroy !");
		}
		Object  iceclassobj = iceclassmap.toObject();
		c_IcePHP_class *cicname = iceclassobj.getTyped<c_IcePHP_class>();
		return cicname;
}

void c_IcePHP_class::t___construct(){
  INSTANCE_METHOD_INJECTION_BUILTIN(IcePHP_class, IcePHP_class::__construct);
  //printf("IcePHP_class object is ok \n");
}

//IcePHP_class_method
IcePHP_Class_method::IcePHP_Class_method(){
} 


IcePHP_Class_method::~IcePHP_Class_method(){
	//	printf("IcePHP_class_method destroy \n");
}

//Proxy
c_Proxy::c_Proxy(const ObjectStaticCallbacks *cb):ExtObjectDataFlags<ObjectData::HasCall>(cb){
		//printf("create a Proxy\n");
}

c_Proxy::~c_Proxy(){
		//printf("Proxy destroy\n");
}

void c_Proxy::t___construct(){
  INSTANCE_METHOD_INJECTION_BUILTIN(Proxy, Proxy::__construct);
  //printf("Proxy object is ok\n");
}

/*
	Ice重要函数，调用Proxy下的魔术函数
*/
Variant c_Proxy::t___call(Variant name,Variant arg){
  INSTANCE_METHOD_INJECTION_BUILTIN(Proxy, Proxy::__call);
  //printf("call method is running \n");
/////////////////////////////////////////////////////////
 Variant result;
///////////////////////////////////////////////////////////
//new ice hzg 20130603
  try{	

	//params process
		  char * name_char=name.toString().data();
		  std::string methodname=name_char;
		  P_ClassMethodPtr method=p_classInfo->methodMap[name_char];
		  if(method){
			  if(method->name!=methodname){
				  throw Exception("method name don't exist !");
			  }
		  }else{
			 raise_error("method object is NULLPOINT!");	
		  }
		  //封装传入的参数，传入到ice中
		  Ice::ByteSeq inParams, outParams;
		  Ice::OutputStreamPtr out = Ice::createOutputStream(communicatorPtr);
		  P_TypeInfoList::iterator q = method->inParams.begin();
		  for (ArrayIter iter(arg); iter; ++iter) {
				  Variant value = iter.second();
				  P_TypeInfoPtr inParamsType = *q;
				  inParamsType->marshal(out,value);
				  if(q != method->inParams.end()){
						q++;
				  }
		  }
		  //如果是传入的类，那么需要绑定对象
		  if(method->sendsClasses){
			 out->writePendingObjects();
		  }
	          out->finished(inParams);
		  ResultCls *rcs=new ResultCls();	
			//调用函数的状态true是成功,false是失败
		  bool status=true;
		  //printf("ice_invoke start\n");
		  if(!objectPrx->ice_invoke(methodname, method->sendMode, inParams, outParams))
		  {
				  status=false;
		  }
		  //捕获ice返回值
		  Ice::InputStreamPtr in = Ice::createInputStream(communicatorPtr, outParams);
		  //如果调用失败，抛出error，解析异常对象
		  if(!status){
				  for(P_ExceptionInfoList::iterator q = method->exceptionList.begin(); q != method->exceptionList.end(); ++q){
						  P_ExceptionInfoPtr exceptionMember = *q;
						  exceptionMember->unmarshal(in,rcs);
						  Object retobj = rcs->result.toObject();
						  delete rcs;
						  rcs = NULL;
						  throw retobj;
				  }

		  }
		  //如果存在返回值，那么对返回的对象进行解封装返回值
		  if(method->returnType){
		  		  
				  method->returnType->unmarshal(in,rcs); 
				  if(method->returnsClasses){
				  	      p_factoryWrapper->setFactory(new P_IceObjectFactory());	
					      in->readPendingObjects();
                  			      p_factoryWrapper->setFactory(0);
				  }
				  method->returnType->unmarshaled(rcs);
				  result=rcs->result;
				 
		  }
		 delete rcs;
		 rcs = NULL;
  }catch(const Ice::Exception& ex)
  {
		  //cerr<<"proxy call error:"<<ex<<endl;
		  raise_error("proxy call error:%s:",ex.what());
  }

    return result;
  /////////////////////////////////////////////////////////
}

Object c_Proxy::t_ice_oneway(){
  INSTANCE_METHOD_INJECTION_BUILTIN(Proxy, Proxy::ice_oneway);
  //printf("ice_oneway is call\n");
	  	objectPrx=objectPrx->ice_oneway();

		return this;
}

Object c_Proxy::t_ice_datagram(){
  INSTANCE_METHOD_INJECTION_BUILTIN(Proxy, Proxy::ice_datagram);
  //printf("ice_datagram is call \n");
  		try{
		objectPrx=objectPrx->ice_datagram();
		}catch(const Ice::Exception& ex){
			//cerr<<"function ice datagram error:"<<ex<<endl;
			raise_error("function ice datagram error:%s:",ex.what());
		}
		return this;
}

Object c_Proxy::t_ice_istwoway(){
  INSTANCE_METHOD_INJECTION_BUILTIN(Proxy, Proxy::ice_istwoway);
  //printf("ice_isTwoway is call \n");	
  		bool b=true;
  		try{
			b=objectPrx->ice_isTwoway();
		}catch(const Ice::Exception& ex){
			//cerr<<"ice_istwoway error:"<<ex<<endl;
			raise_error("ice_istwoway error:%s:",ex.what());
		}
		return this;
}

Object c_Proxy::t_ice_secure(bool secure /*= false*/){
  INSTANCE_METHOD_INJECTION_BUILTIN(Proxy, Proxy::ice_secure);
  //printf("ice_secure is call \n");
		objectPrx=objectPrx->ice_secure(secure);
		return this;
}

Object c_Proxy::t_ice_timeout(int64 lasttime){
  INSTANCE_METHOD_INJECTION_BUILTIN(Proxy, Proxy::ice_timeout);
  //printf("ice_timeout is call \n");
		//cout<<(int)lasttime<<"timeout"<<endl;
	     try{
			objectPrx=objectPrx->ice_timeout((int)lasttime);
		}catch(const Ice::Exception& ex){
			//cerr<<"ice_timeout error:"<<ex<<endl;
			raise_error("ice_timeout error:%s:",ex.what());
		}

		return this;
}

Object c_Proxy::t_ice_context(CArrRef ctx){
  INSTANCE_METHOD_INJECTION_BUILTIN(Proxy, Proxy::ice_context);
  //printf("ice_context is call \n");
		Ice::Context ice_ctx;
		for (ArrayIter iter(ctx); iter; ++iter) {
				Variant key=iter.first();
				Variant value = iter.second();
				std::string ctx_key=key.toString().data();
				ice_ctx[ctx_key] = value.toString();
		}
		try{
		objectPrx=objectPrx->ice_context(ice_ctx);
		}catch(const Ice::Exception& ex){
			//cerr<<"ice context error:"<<ex<<endl;
			raise_error("ice context error:%s:",ex.what());
		}
		return this;
}

Object c_Proxy::t_ice_checkedcast(CStrRef classid, CVarRef facetOrCtx /*= null*/, CVarRef ctx /*= null*/){
  INSTANCE_METHOD_INJECTION_BUILTIN(Proxy, Proxy::ice_checkedcast);
  //printf("ice_checkedcast is call \n");
		std::string id = classid.data();
		P_ClassInfoPtr p_classInfoPtr=getP_ClassInfoPtrById(id);
		p_classInfo=p_classInfoPtr;
		if(facetOrCtx){
				//objectPrx = objectPrx->ice_facet(facetOrCtx.toCArrRef());
		}

		if(ctx){
				Ice::Context ice_ctx;
				for (ArrayIter iter(ctx); iter; ++iter) {
						Variant key=iter.first();
						Variant value = iter.second();
						std::string ctx_key=key.toString().data();
						ice_ctx[ctx_key] = value.toString();
				}
				objectPrx=objectPrx->ice_context(ice_ctx);
		}

		return this;
}
//
Object c_Proxy::t_ice_uncheckedcast(CStrRef classid, CVarRef facet /*= null*/){
  INSTANCE_METHOD_INJECTION_BUILTIN(Proxy, Proxy::ice_uncheckedcast);
  //printf("ice_uncheckedcast is call \n");
		std::string id = classid.data();
		P_ClassInfoPtr p_classInfoPtr=getP_ClassInfoPtrById(id);
		p_classInfo=p_classInfoPtr;

		if(facet){
				//objectPrx = objectPrx->ice_facet(facetOrCtx.toCArrRef());
		}

		return this;
}

/*
	在Ice.php中定义基本类行的函数
*/
Variant f_create_typeinfobyid(int id /* = 0 */) {
		c_TypeInfo *typeObj=NULL;
		switch(id){
				case 0:
						typeObj = NEWOBJ(PrimitiveInfo)(PrimitiveInfo::KindString);
						//typeObj=new PrimitiveInfo(PrimitiveInfo::KindString);
						break;
				case 1:
						typeObj = NEWOBJ(PrimitiveInfo)(PrimitiveInfo::KindLong);
						//typeObj=new PrimitiveInfo(PrimitiveInfo::KindLong);
						break;
				case 2:
						typeObj = NEWOBJ(PrimitiveInfo)(PrimitiveInfo::KindInt);
						//typeObj=new PrimitiveInfo(PrimitiveInfo::KindInt);
						break;
				case 3:
						typeObj = NEWOBJ(Ice_ClassInfo)();
						//typeObj=new Ice_ClassInfo();
						break;
				case 4:
						typeObj = NEWOBJ(PrimitiveInfo)(PrimitiveInfo::KindDouble);
						//typeObj=new PrimitiveInfo(PrimitiveInfo::KindDouble);
						break;
				case 5:
						c_IcePHP_class *ic=NEWOBJ(c_IcePHP_class)();
						//c_IcePHP_class *ic=new c_IcePHP_class();
						return Variant(ic);
		}
		return Variant(typeObj);
}

/*
	定义异常类型的函数
	将名称，成员等信息封装到P_ExceptionInfoPtr中，并返回ExceptionInfo对象
*/
Object f_icephp_defineexception(CStrRef id /* = null */, CStrRef name /* = null */, CVarRef base /* = null */, CVarRef members /* = null */) {
		ExceptionInfo* ex = NEWOBJ(ExceptionInfo)();
		ex->id = id.data();

        	///////////wh add at 20130530/////////////////
        	P_ExceptionInfoPtr pcp = c_TypeInfoToP_TypeInfoPtr(ex);
		pcp->clearDataMemberList();
        	pcp->id = ex->id;
        	pcp->name = name.data();   
        	///////////////////////////////////////////////		
			
		if(members){
				//DataMemberList dlist;
				/////////wh add 20130530///////////////
				P_DataMemberList p_datamemberList;
				///////////////////////////////////////

				Array memArray = members.toArray();
				for(int i = 0; i < memArray.size(); i++){

						////wanghong add at 20130605///////////
						std::string dataMemberName = getHPHPStringToStdString(memArray[i][0].toString());
						c_TypeInfo* ei = (memArray[i][1].toObject()).getTyped<c_TypeInfo>()->getc_type();
						///////////////////////////////////////

						/////////wh add 20130530///////////////
						P_DataMemberPtr p_datamember = new P_DataMember() ;
						p_datamember->name = dataMemberName;//p_datamember->name = dmp->name ;
						p_datamember->type = c_TypeInfoToP_TypeInfoPtr(ei) ;
						p_datamemberList.push_back(p_datamember) ;
						///////////////////////////////////////

				}
				/////////wh add 20130530///////////////
				pcp->members = p_datamemberList;
				///////////////////////////////////////

		}	

		/////////wh add 20130530///////////////
		pcp->p_usesClasses = false;
		///////////////////////////////////////
		return ex;
}

/*
	打印异常对象的信息
	ice_name是在Ice生成模板中生成的异常的id，但是前面需要加上::才是完成的Id,这个捕获是在getP_ExceptionInfoPtrById中实现的
*/
String f_icephp_stringifyexception(CObjRef value /* = null */, CObjRef target /* = null */) {
	if(f_method_exists(value,"ice_name")){
		Variant returnValue=f_call_user_method(0,"ice_name",value);		
		std::string exceptionId=getHPHPStringToStdString(returnValue.toString());
		P_ExceptionInfoPtr p_exceptionPtr=getP_ExceptionInfoPtrById(exceptionId);
		Variant v=Variant(value);
		return  p_exceptionPtr->print(0,v);
	}else{
		raise_warning("Exception ice_name  is not exists!");		
	}
	return "";
}

/*
	打印类对象的信息
	ice_staticId是生成的Ice模板中的Id信息，class和sequnce都有这个函数获取ID信息
*/
String f_icephp_stringify(CObjRef value /* = null */, CObjRef target /* = null */) {
	if(f_method_exists(value,"ice_staticId")){
		Variant returnValue=f_call_user_method(0,"ice_staticId",value);		
		std::string classId=getHPHPStringToStdString(returnValue.toString());
		P_ClassInfoPtr p_classInfoPtr=getP_ClassInfoPtrById(classId);
		Variant v=Variant(value);
		return  p_classInfoPtr->print(0,v);
	}else{
		raise_warning("class ice_staticId is not exists!");		
	}
	return "";
  }


Object f_icephp_definesequence(CStrRef id /* = null */, CVarRef elementtype /* = null */) {
		//SequenceInfo* type = new SequenceInfo();
		if(elementtype.isNull()){
			return NULL;
		}
		SequenceInfo* type = NEWOBJ(SequenceInfo)();
		type->id = id;

		////wh add 20130530//////////////////////////////////
		P_SequenceInfoPtr psi = c_TypeInfoToP_TypeInfoPtr(type);
		psi->id = id;//psi->id = type->id;

		////wh add at 20130605////////////////
		c_TypeInfo* ti = elementtype.toObject().getTyped<c_TypeInfo>();
		//////////////////////////////////////

		P_TypeInfoPtr pti = c_TypeInfoToP_TypeInfoPtr(ti->getc_type());
		psi->elementType = pti;
		/////////////////////////////////////////////////////

		return type;
}


Object f_icephp_defineproxy(CObjRef classobj) {
	return NULL;
}

Object f_icephp_declareclass(CStrRef id /* = null */){
	return NULL;
}

Object f_icephp_defineclass(CStrRef id, CStrRef name, bool isabstract /* = true */, CObjRef base /* = null */, CVarRef interfaces /* = null */, CVarRef members /* = null */) {
	Ice_ClassInfo* ci = NEWOBJ(Ice_ClassInfo)();
	ci->id = id.data();
	///20130529 new struct////////////////////////////////////////////
	P_ClassInfoPtr pcp=c_TypeInfoToP_TypeInfoPtr(ci);
	pcp->clearDataMemberList();
        pcp->id=ci->id;
	pcp->name=name.data();
	pcp->isAbstract=isabstract;	
	//////////////////////////////////////////////
	if(members){     
		//DataMemberList dlist;
		////////20130529 new struct///////////////////////
		 P_DataMemberList p_datamemberlist;
		/////////////////////////////
		Array mem = members.toArray();
		for(int i=0 ; i<mem.size() ; i++){
			std::string dataMemberName= getHPHPStringToStdString(mem[i][0].toString());
			c_TypeInfo* ti = (mem[i][1].toObject()).getTyped<c_TypeInfo>();
			////////20130529 new struct///////////////////////
		        P_DataMemberPtr p_datamember=new P_DataMember();	
			p_datamember->name=dataMemberName;
			p_datamember->type=c_TypeInfoToP_TypeInfoPtr(ti);		
			p_datamemberlist.push_back(p_datamember);
			////////////////////////////////////////////////
		}
		////hzg new ice//////////////////////////////////
	        pcp->members=p_datamemberlist;	
		/////////////////////////////////////
	}
	ci->p_typeInfo=pcp;
	return ci;
}

//
void f_icephp_defineoperation(CObjRef classobj /* = null */, CStrRef funname /* = null */, int mode /* = null */, int sendmode /* = null */, CVarRef inparams /* = null */, CVarRef outparams /* = null */, CVarRef returntype /* = null */, CVarRef exceptions /* = null */) {
		//printf("defineoperation is call \n");
		IcePHP_Class_method *icm = new IcePHP_Class_method();
		///////hzg edit////////////////////////////////////////////////////////////
		Ice_ClassInfoPtr cic=classobj.getTyped<Ice_ClassInfo>();
		/////////////////////////////////////////////////////////////////////
		////hzg new ice///////////////////////////////////////////////////////
		std::string methodName= funname.data();
		P_ClassInfoPtr pcp=c_TypeInfoToP_TypeInfoPtr(cic);
		P_ClassMethodMap methodMap=pcp->methodMap;
		P_ClassMethodMap::iterator methodIterator; 
		methodIterator=methodMap.find(methodName);
		P_ClassMethodPtr classMethod=NULL;
		if(methodIterator!=methodMap.end()){
			classMethod=methodIterator->second;
		}else{
			classMethod=new P_ClassMethod();
			methodMap[methodName]=classMethod;
			pcp->methodMap=methodMap;
		}
		classMethod->name=methodName;
		classMethod->mode=mode;
		classMethod->sendMode=sendmode;	
		///////////////////////////////////////////////////////////
		if(inparams){
				Array inArray = inparams.toArray();
				TypeInfoList tif;
				////hzg new ice///////////////////////////////////////////////////////
				P_TypeInfoList p_inParams;
				 /////////////////////////////////////////////////////////////////////
				for(int i = 0; i < inArray.size() ; i++){
						Object inpo = inArray[i].toObject();
						c_TypeInfo* paramType = inpo.getTyped<c_TypeInfo>();
						////hzg new ice///////////////////////////////////////////////////////
						P_TypeInfoPtr p_typeInfoPtr=c_TypeInfoToP_TypeInfoPtr(paramType->getc_type());
						if(!classMethod->sendsClasses){
							 classMethod->sendsClasses= p_typeInfoPtr->usesClasses();
						}
						//classMethod->sendsClasses=icm->sendsClasses;	
						p_inParams.push_back(p_typeInfoPtr);
						/////////////////////////////////////////////////////////////////////
				}
				////hzg new ice///////////////////////////////////////////////////////
				classMethod->inParams=p_inParams;	
				/////////////////////////////////////////////////////////////////////

		}

		if(outparams){
				raise_error("don't support outparams in method!");
		}

		if(returntype){
				Object rttype = returntype.toObject();
				c_TypeInfo* rt = rttype.getTyped<c_TypeInfo>();

				////hzg new ice///////////////////////////////////////////////////////
				P_TypeInfoPtr p_returnType=c_TypeInfoToP_TypeInfoPtr(rt);
				classMethod->returnType=p_returnType;	
				classMethod->returnsClasses=p_returnType->usesClasses();
				/////////////////////////////////////////////////////////////////////
		}

		if(exceptions){
				ExceptionInfoList eil;
				////hzg new ice///////////////////////////////////////////////////////
				P_ExceptionInfoList p_exceptionList;
				/////////////////////////////////////////////////////////////////////
				Array exp = exceptions.toArray();
				for(int i = 0 ; i < exp.size() ; i++ ){
						Object expo = exp[i].toObject();
						ExceptionInfo* ei = expo.getTyped<ExceptionInfo>();
						////hzg new ice///////////////////////////////////////////////////////
						p_exceptionList.push_back(c_TypeInfoToP_TypeInfoPtr(ei));	
						/////////////////////////////////////////////////////////////////////
				}
				////hzg new ice///////////////////////////////////////////////////////
				classMethod->exceptionList=p_exceptionList;	
				/////////////////////////////////////////////////////////////////////

		}
		//delete old ice code hzg 20130605	
		//(cic->methodMap)[funname.data()]=icm;

}

c_Proxy* ProxyInfo::getProxyInfo(std::string proxyInfo){
		std::string std_classId=proxyInfo;
		ProxyMap::iterator p = proxyMap.find(proxyInfo);
		if( p != proxyMap.end()){
				return p->second;//how can an object covert to point 
		}else{
				return NULL;//there has error,if doesn't exist point return a null point.
		}
}

ProxyInfo * HPHP::ProxyInfo::m_pInstance1=NULL;

ProxyInfo* ProxyInfo::GetInstance(){
		if(m_pInstance1 == NULL)
				m_pInstance1 = new ProxyInfo();
		return m_pInstance1;
}


// UnmarshalCallback implementation
UnmarshalCallback::~UnmarshalCallback(){

}

// TypeInfo implementation.
c_TypeInfo::c_TypeInfo(const ObjectStaticCallbacks *cb):ExtObjectData(cb){
		//printf("TypeInfo is running \n");
}

c_TypeInfo::~c_TypeInfo(){
		//printf("TypeInfo is destroy \n");
}

c_TypeInfo *c_TypeInfo::getc_type(){
		c_TypeInfo *c_typeInfoObj;
		switch(c_kind){
				case c_TypeInfo::PrimitiveInfo:
						{
								c_typeInfoObj = dynamic_cast<HPHP::PrimitiveInfo *>(this); 	
								//this = this->getTyped<PrimitiveInfo>();
								break;
						}
				case c_TypeInfo::Ice_ClassInfo:
						{
								c_typeInfoObj = dynamic_cast<HPHP::Ice_ClassInfo *>(this);
								//this = this->getTyped<Ice_ClassInfo>();
								break;
						}
				case c_TypeInfo::SequenceInfo:
						{
								c_typeInfoObj = dynamic_cast<HPHP::SequenceInfo *>(this);
								break;
						}
				case c_TypeInfo::ExceptionInfo:
						{
								c_typeInfoObj = dynamic_cast<HPHP::ExceptionInfo *>(this);
								break;
						}
				default:
						break;
		}
		return c_typeInfoObj;
		//return this;
}

void c_TypeInfo::t___construct(){
  INSTANCE_METHOD_INJECTION_BUILTIN(TypeInfo, TypeInfo::__construct);
  //printf("typeinfo is construct \n");
}

bool c_TypeInfo::usesClasses()
{
		return false;
}

// PrimitiveInfo implementation.
PrimitiveInfo::PrimitiveInfo(int type){
		kind = type;
		c_kind = c_TypeInfo::PrimitiveInfo;
}

// PrimitiveInfo implementation.
string PrimitiveInfo::getId() const {
		switch(kind)
		{    
				case KindBool:
						return "bool";
				case KindByte:
						return "byte";
				case KindShort:
						return "short";
				case KindInt:
						return "int";
				case KindLong:
						return "long";
				case KindFloat:
						return "float";
				case KindDouble:
						return "double";
				case KindString:
						return "string";
		}
		return string();
}


// ClassInfo implementation.
string Ice_ClassInfo::getId() const {
		return id;
}

//////////////////////////////////////////////////////////////////////////////
//Ice stream
IceObjectReader::IceObjectReader(const string &classId){
}

IceReadObjectCallback::IceReadObjectCallback(const string className,ResultCls * rcs){
}

IceObjectFactory::IceObjectFactory(){
}

IceObjectWriter::IceObjectWriter(const Ice_ClassInfoPtr &classInfo,Variant &inParams){
}

/////////////////////////////////////////////////////////////////////////////////
ExceptionInfo::ExceptionInfo(){
	c_kind=c_TypeInfo::ExceptionInfo;
}

ExceptionInfo:: ~ExceptionInfo(){
}

Ice_ClassInfoPtr getClassInfoPtrById(std::string classId){
		std::map<std::string,Ice_ClassInfoPtr>::iterator l_it;
		c_IcePHP_class *cicname = get_hiphopice_classmap();
		l_it=(cicname->classInfoMap).find(classId);
		if(l_it!=(cicname->classInfoMap).end()){
				return l_it->second;
		}
		return NULL;

};
ExceptionInfoPtr getExceptionInfoPtrById(std::string exceptionId){
		return NULL;	
};

std::string getHPHPStringToStdString(HPHP::String hphpstring){
		char * std_char=hphpstring.data();
		std::string std_string=std_char;
		return std_string;
}


SequenceInfo::SequenceInfo()
{
		c_kind = c_TypeInfo::SequenceInfo;	
};


char * getCurrentTid(){
		pthread_t tid;
		tid = pthread_self();
		Variant tidv=Variant((int64)tid);
		char * tidc=tidv.toString().data();
		return  tidc;
}

Ice_Communicator * getIce_CommunicatorByMap(){
		char * tidc=getCurrentTid();
		Ice_Communicator *ic=dynamic_cast<Ice_Communicator*>(g_persistentObjects->get(Communicator_persistentObject_name, tidc));
		return ic;
}


///20130529 new struct///////////////////////////////////////////////////////////////

P_TypeInfoPtr c_TypeInfoToP_TypeInfoPtr(c_TypeInfo* typeInfo){	
	IceStoreMsg *ism=NULL;
	P_TypeInfoPtr p_typeInfo=NULL;
	switch(typeInfo->c_kind){
		case c_TypeInfo::PrimitiveInfo:
			{
				HPHP::PrimitiveInfo * c_typeInfoObj = dynamic_cast<HPHP::PrimitiveInfo *>(typeInfo); 	
				char* primitiveId=c_typeInfoObj->getId().c_str();
				ism=dynamic_cast<IceStoreMsg*>(g_persistentObjects->get(PrimitiveInfo_Map_Name,primitiveId));
				if(ism){
					p_typeInfo=ism->typeInfo;	
				}else{
					p_typeInfo=new P_PrimitiveInfo(c_typeInfoObj->kind);
					ism=new IceStoreMsg();	
					ism->typeInfo=p_typeInfo;
					g_persistentObjects->set(PrimitiveInfo_Map_Name,primitiveId,ism);
				}	
				break;
			}
		case c_TypeInfo::Ice_ClassInfo:
			{
				HPHP::Ice_ClassInfo *c_typeInfoObj=NULL;
				c_typeInfoObj = dynamic_cast<HPHP::Ice_ClassInfo *>(typeInfo);
				char * typeId=c_typeInfoObj->id.c_str();
				ism=dynamic_cast<IceStoreMsg*>(g_persistentObjects->get(ClassInfo_Map_Name,typeId));
				if(ism){
					p_typeInfo=ism->typeInfo;	
				}else{
					p_typeInfo=new P_ClassInfo();
					p_typeInfo->id=c_typeInfoObj->id;
					ism=new IceStoreMsg();	
					ism->typeInfo=p_typeInfo;
					g_persistentObjects->set(ClassInfo_Map_Name,typeId,ism);
				}	
				break;
			}
		case c_TypeInfo::SequenceInfo:
			{
				HPHP::SequenceInfo *c_typeInfoObj=NULL;
				c_typeInfoObj = dynamic_cast<HPHP::SequenceInfo *>(typeInfo);
				char * typeId=c_typeInfoObj->id.c_str();
				ism=dynamic_cast<IceStoreMsg*>(g_persistentObjects->get(SequenceInfo_Map_Name,typeId));
				if(ism){
					p_typeInfo=ism->typeInfo;	
				}else{
					p_typeInfo=new P_SequenceInfo();
					p_typeInfo->id=c_typeInfoObj->id;
					ism=new IceStoreMsg();	
					ism->typeInfo=p_typeInfo;
					g_persistentObjects->set(SequenceInfo_Map_Name,typeId,ism);
				}	
				break;
			}
		case c_TypeInfo::ExceptionInfo:
			{
				HPHP::ExceptionInfo *c_typeInfoObj=NULL;
				c_typeInfoObj = dynamic_cast<HPHP::ExceptionInfo *>(typeInfo);
				char * typeId=c_typeInfoObj->id.c_str();
				ism=dynamic_cast<IceStoreMsg*>(g_persistentObjects->get(ExceptionInfo_Map_Name,typeId));
				if(ism){
					p_typeInfo=ism->typeInfo;	
				}else{
					p_typeInfo=new P_ExceptionInfo();
					p_typeInfo->id=c_typeInfoObj->id;
					ism=new IceStoreMsg();	
					ism->typeInfo=p_typeInfo;
					g_persistentObjects->set(ExceptionInfo_Map_Name,typeId,ism);
				}	

				break;
			}
		default:
			break;
	}

	return p_typeInfo;
}


std::string P_PrimitiveInfo::getP_PrimitiveInfoType(int type){
	std::string primitiveId="";
	switch(type){
		case P_PrimitiveInfo::KindBool:
			primitiveId="P_PrimitiveInfo::KindBool";
			break;
		case P_PrimitiveInfo::KindByte:
			primitiveId="P_PrimitiveInfo::KindByte";	
			break;
		case P_PrimitiveInfo::KindShort:
			primitiveId="P_PrimitiveInfo::KindShort";	
			break;
		case P_PrimitiveInfo::KindInt:
			primitiveId="P_PrimitiveInfo::KindInt";	
			break;
		case P_PrimitiveInfo::KindLong:
			primitiveId="P_PrimitiveInfo::KindLong";	
			break;
		case P_PrimitiveInfo::KindFloat:
			primitiveId="P_PrimitiveInfo::KindFloat";	
			break;
		case P_PrimitiveInfo::KindDouble:
			primitiveId="P_PrimitiveInfo::KindDouble";	
			break;
		case P_PrimitiveInfo::KindString:
			primitiveId="P_PrimitiveInfo::KindString";	
			break;
	}
	return primitiveId;
}

void P_ClassInfo::clearDataMemberList(){
	for(P_DataMemberList::const_iterator memberIter = members.begin(); memberIter != members.end(); ++memberIter){
		P_DataMemberPtr memberPtr= *memberIter;
		delete memberPtr;
		memberPtr=NULL;
	}
}
void P_ExceptionInfo::clearDataMemberList(){
        for(P_DataMemberList::const_iterator memberIter = members.begin(); memberIter != members.end(); ++memberIter){
                P_DataMemberPtr memberPtr= *memberIter;
                delete memberPtr;
                memberPtr=NULL;
        }
}
///////////////////////////////////////////////////////////////////////////////////
void P_ClassInfo::marshal(const Ice::OutputStreamPtr& out,Variant &inParams){
	P_IceObjectWriterPtr writer = new P_IceObjectWriter(this,inParams);
	out->writeObject(writer);
}

void P_ClassInfo::unmarshal(const Ice::InputStreamPtr& in,ResultCls * rcs){
	in->readObject(new P_IceReadObjectCallback(this->name,rcs));
}
void P_ClassInfo::unmarshaled(ResultCls *result){
	if(result->flag){
		ResultList list=result->next; 
		ResultList::iterator itr;
		itr=list.begin();
		Array params=Array::Create();
		for(P_DataMemberList::iterator q = members.begin(); q != members.end(); ++q){
			P_DataMemberPtr member = *q;
			ResultCls *rcs=*itr;
			if(rcs->flag){
				member->type->unmarshaled(rcs);
				params.appendRef(rcs->result);
			}else{
				params.appendRef(Variant());
			}
			if(itr!=list.end()){
				itr++;
			}
		}
		Object returnObj=create_object(name,params,true);
		result->result=Variant(returnObj);	
	}else{
		result->result=Variant();
	}
}
string  P_ClassInfo::print(int objcount,Variant &inParams){
	if(inParams.isNull()){
		return "";
	}
	string printValue="";
	Variant classVars=f_get_object_vars(inParams);
	String count=String(objcount);
	printValue.append("object #");
	printValue.append(count);
	printValue.append(" (");
	printValue.append(id);
	printValue.append(" ) ");
	printValue.append(" { ");
	for(P_DataMemberList::iterator q = members.begin(); q != members.end(); ++q){
		P_DataMemberPtr member = *q;
		std::string dataMemberName=member->name;
		printValue+=dataMemberName+"=";
		Variant dataMemberValue=classVars[Variant(dataMemberName)];
		objcount++;
		printValue+=member->type->print(objcount,dataMemberValue);
		printValue+="\n";
	}
	printValue+="}";
	return printValue;
}

//////////////////////////////////////////////////////////////////////////////
//Ice stream
P_IceObjectReader::P_IceObjectReader(const string &classId){
		this->classInfo=getP_ClassInfoPtrById(classId);
}

void P_IceObjectReader::read(const Ice::InputStreamPtr& in, bool rid){
		if(rid)
		{
				in->readTypeId();
		}

		in->startSlice();
		for(P_DataMemberList::iterator p = classInfo->members.begin(); p != classInfo->members.end(); ++p)
		{
				P_DataMemberPtr member = *p;
				ResultCls *rl=new ResultCls();
				member->type->unmarshal(in,rl);
				resultList.push_back(rl);
		}
		in->endSlice();
		::Ice::Object::__read(in, true);
}

P_IceReadObjectCallback::P_IceReadObjectCallback(const string className,ResultCls * rcs){
		this->className=className;
		this->rcs=rcs;
}

void P_IceReadObjectCallback::invoke(const Ice::ObjectPtr& p){
		if(p){
			P_IceObjectReaderPtr reader=P_IceObjectReaderPtr::dynamicCast(p);
			ResultList resultList=reader->resultList;
			rcs->next=resultList;
		}else {
			rcs->flag=false;		
		}
}

P_IceObjectFactory::P_IceObjectFactory(){
}

Ice::ObjectPtr P_IceObjectFactory::create(const string& type){
		return new P_IceObjectReader(type);
}

Ice::ObjectPtr P_IceObjectFactoryWrapper::create(const string& type){
		return _factory->create(type);
}

void P_IceObjectFactoryWrapper::setFactory(const Ice::ObjectFactoryPtr& factory){
		_factory = factory;
}

P_IceObjectWriter::P_IceObjectWriter(const P_ClassInfoPtr &classInfo,Variant &inParams){
		this->classInfo=classInfo;
		this->inParams=inParams;
}

void P_IceObjectWriter::write(const Ice::OutputStreamPtr& out) const{	
		out->writeTypeId(classInfo->id);
		Variant classObj=inParams;
		Variant classVars;
		if(!classObj.isNull()){
			classVars=f_get_object_vars(classObj);		
		}else{
			classVars=Variant();
		}
		out->startSlice();
		for(P_DataMemberList::iterator q = classInfo->members.begin(); q != classInfo->members.end(); ++q){
				P_DataMemberPtr member = *q;
				Variant dataMemberValue=classVars[Variant(member->name)];
				member->type->marshal(out,dataMemberValue);
		}
		out->endSlice();
		::Ice::Object::__write(out);
}

P_ClassInfoPtr getP_ClassInfoPtrById(const string &classId){
	char * typeId=classId.c_str();
	IceStoreMsg *ism=dynamic_cast<IceStoreMsg*>(g_persistentObjects->get(ClassInfo_Map_Name,typeId));
	if(ism){
		P_ClassInfoPtr p_ClassInfo=dynamic_cast<P_ClassInfoPtr>(ism->typeInfo);	
		return p_ClassInfo;
	}
	raise_error("classid don't register!classInfo is NULLPOINT!");
	return NULL;

}

void P_PrimitiveInfo::marshal(const Ice::OutputStreamPtr& out,Variant &inParams){
	HPHP::P_PrimitiveInfo *piObj = dynamic_cast<HPHP::P_PrimitiveInfo *>(this);
	pkind=piObj->pkind;
	switch(pkind){
		case P_PrimitiveInfo::KindString:
			out->write(getHPHPStringToStdString(inParams.toString()));
			break;
		case P_PrimitiveInfo::KindInt:
			out->write(static_cast<Ice::Int>(inParams.toInt32()));
			break;
		case P_PrimitiveInfo::KindLong:
			out->write(static_cast<Ice::Long>(inParams.toInt64()));
			break;
		case P_PrimitiveInfo::KindDouble:
			out->write(static_cast<Ice::Double>(inParams.toDouble()));
			break;
	}	
}
void P_PrimitiveInfo::unmarshal(const Ice::InputStreamPtr& in,ResultCls * rcs){
	Variant primitiveValue;
	switch(pkind){
		case P_PrimitiveInfo::KindString:
			{
				std::string value;
				in->read(value);
				primitiveValue=Variant(value);
				break;
			}
		case P_PrimitiveInfo::KindInt:
			{
				int value;
				in->read(value);
				primitiveValue=Variant(value);
				break;
			}
		case P_PrimitiveInfo::KindLong:
			{
				Ice::Long value;
				in->read(value);
				primitiveValue=Variant(value);
				break;
			}
		case P_PrimitiveInfo::KindDouble:
			{
				double value;
				in->read(value);
				primitiveValue=Variant(value);
				break;
			}
	}
	rcs->result=primitiveValue;
}
void P_PrimitiveInfo::unmarshaled(ResultCls *result){

}
string P_PrimitiveInfo:: print(int objcount,Variant &inParams){
	return getHPHPStringToStdString(inParams.toString());
}

void P_ExceptionInfo::marshal(const Ice::OutputStreamPtr& out,Variant &inParams){
	
}
void P_ExceptionInfo::unmarshal(const Ice::InputStreamPtr& in,ResultCls * rcs){
	bool p_usesClasses;
	in->read(p_usesClasses);
	string id;
	in->read(id);
	in->startSlice();
	Array params= Array::Create();

	for(P_DataMemberList::iterator q = members.begin(); q != members.end(); ++q){
		P_DataMemberPtr member = *q;
		ResultCls * memberValue=new ResultCls();
		member->type->unmarshal(in,memberValue);
		params.appendRef(memberValue->result);
		delete memberValue;
		memberValue = NULL;
	}
	in->endSlice();	
	Object resultObj=create_object(name,params,true);
	rcs->result=Variant(resultObj);
}
void P_ExceptionInfo::unmarshaled(ResultCls *result){

}
string P_ExceptionInfo::print(int objcount,Variant &inParams){
	string printValue="";
	Variant classVars=f_get_object_vars(inParams);
	for (ArrayIter iter(classVars.toArray()); iter; ++iter) {
		Variant vKey = iter.first();
		Variant vValue = iter.second();
	}
	printValue+="exception  "+id+"{";
	for(P_DataMemberList::iterator q = members.begin(); q != members.end(); ++q){
		P_DataMemberPtr member = *q;
		std::string dataMemberName=member->name;
		printValue+=dataMemberName+"=";
		Variant dataMemberValue=classVars[Variant(dataMemberName)];
		objcount++;
		printValue+=member->type->print(objcount,dataMemberValue);
	}
	printValue+="}";
	return printValue;
}
void P_SequenceInfo::marshal(const Ice::OutputStreamPtr& out,Variant &inParams){
	HPHP::P_PrimitiveInfo *pi= dynamic_cast<HPHP::P_PrimitiveInfo *>(elementType);	
	if(pi){
		marshalPrimitiveSequence(out,inParams,pi);
		return ;
	}
	Array inParamArray=inParams.toArray();
	int writeSize=inParamArray.size();	
	out->writeSize(writeSize);
	for (ArrayIter iter(inParamArray); iter; ++iter) {
		Variant inParam = iter.second();
		elementType->marshal(out,inParam);
	}
}
void P_SequenceInfo::marshalPrimitiveSequence(const Ice::OutputStreamPtr& out,Variant &inParams,HPHP::P_PrimitiveInfo *primitiveInfo){
	Array inParamArray=inParams.toArray();
	switch(primitiveInfo->pkind){
		case P_PrimitiveInfo::KindString:
		{
			std::vector< std::string> sequenceList;
			for (ArrayIter iter(inParamArray); iter; ++iter) {
				Variant inParam = iter.second();
				sequenceList.push_back(getHPHPStringToStdString(inParam.toString()));
			}
			out->write(sequenceList);
			break;	
		}
		case P_PrimitiveInfo::KindInt:
		{
			std::vector< ::Ice::Int> sequenceList;
			for (ArrayIter iter(inParamArray); iter; ++iter) {
					Variant inParam = iter.second();	
				    sequenceList.push_back(static_cast<Ice::Int>(inParam.toInt32()));	
			}
			out->write(sequenceList);
			break;			
		}
		case P_PrimitiveInfo::KindLong:
		{
			std::vector< ::Ice::Long> sequenceList;
			for (ArrayIter iter(inParamArray); iter; ++iter) {
					Variant inParam = iter.second();	
				    sequenceList.push_back(static_cast<Ice::Long>(inParam.toInt64()));	
			}
			out->write(sequenceList);
			break;			
		}
		case P_PrimitiveInfo::KindDouble:
		{
			std::vector< ::Ice::Double> sequenceList;
			for (ArrayIter iter(inParamArray); iter; ++iter) {
					Variant inParam = iter.second();
					sequenceList.push_back(static_cast<Ice::Long>(inParam.toDouble()));
			}
			out->write(sequenceList);
			break;
		}
	}	
}
void P_SequenceInfo::unmarshal(const Ice::InputStreamPtr& in,ResultCls * rcs){
	::Ice::Int sz = in->readAndCheckSeqSize(4);
	for(int i=0;i<sz;i++){
		Variant sequenceValue;
		ResultCls *resultCls=new ResultCls();
		resultCls->result=sequenceValue;
		elementType->unmarshal(in,resultCls);
		rcs->next.push_back(resultCls);
	}
}
void P_SequenceInfo::unmarshaled(ResultCls *result){
	ResultList list=result->next;
	ResultList::iterator itr;
//	Variant sequenceList;
	Array sequenceList= Array::Create();
	for(itr=list.begin();itr!=list.end();itr++){
		ResultCls *rcs=*itr;
		elementType->unmarshaled(rcs);
	//	sequenceList.append(rcs->result);
		sequenceList.appendRef(rcs->result);
	}
//	result->result=sequenceList;
	result->result=Variant(sequenceList);
}
string P_SequenceInfo::print(int objcount,Variant &inParams){
	return "";
}

P_ExceptionInfoPtr getP_ExceptionInfoPtrById(const string &exceptionId){
	exceptionId="::"+exceptionId;
	char * typeId=exceptionId.c_str();
	IceStoreMsg *ism=dynamic_cast<IceStoreMsg*>(g_persistentObjects->get(ExceptionInfo_Map_Name,typeId));
	if(ism){
		P_ExceptionInfoPtr p_ExceptionInfo=dynamic_cast<P_ExceptionInfoPtr>(ism->typeInfo);	
		return p_ExceptionInfo;
	}
	raise_error("exceptionId don't register!ExceptionInfo is NULLPOINT!");
	return NULL;
}
///////////////////////////////////////////////////////////////////////////////
}

