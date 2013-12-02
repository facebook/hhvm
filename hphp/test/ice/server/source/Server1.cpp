// **********************************************************************
//
// Copyright (c) 2003-2010 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <class_demoI.h>

using namespace std;

class ClassDemoServer : public Ice::Application
{
public:

    virtual int run(int, char*[]);
};

int
main(int argc, char* argv[])
{
		int status;
		Ice::CommunicatorPtr communicator;

		try
		{
				communicator = Ice::initialize(argc, argv);
						//	communicator()->getProperties()->setProperty("ClassDemo.ThreadPool.Size", "100");
				communicator->getProperties()->setProperty("ClassDemo.Endpoints","tcp -p 10002:udp -p 10002");
				communicator->getProperties()->setProperty("Ice.ThreadPool.Server.Size", "1");

				communicator->getProperties()->setProperty("ClassDemo.ThreadPool.Size", "1");
				//communicator->getProperties()->setProperty("Ice.ThreadPool.Server.ThreadPriority", "10");
				//communicator->getProperties()->setProperty("ClassDemo.ThreadPool.ThreadPriority", "50");
				Ice::ObjectAdapterPtr adapter = communicator->createObjectAdapter("ClassDemo");
				com::jd::CommontIceRpcServicePtr classdemo = new com::jd::CommontIceRpcServiceI;

				adapter->add(classdemo, communicator->stringToIdentity("classdemo"));
				adapter->activate();
				communicator->waitForShutdown();

		}
		catch(const Ice::Exception& ex)
		{
				cerr << ex << endl;
				status = EXIT_FAILURE;
		}

		if(communicator)
		{
				try
				{
						communicator->destroy();
				}
				catch(const Ice::Exception& ex)
				{
						cerr << ex << endl;
						status = EXIT_FAILURE;
				}
		}


//	ClassDemoServer app;
//    return app.main(argc, argv, "config1.server");
	return 0;
}

int
ClassDemoServer::run(int argc, char* argv[])
{
    if(argc > 1)
    {
        cerr << appName() << ": too many arguments" << endl;
        return EXIT_FAILURE;
    }

    Ice::ObjectAdapterPtr adapter = communicator()->createObjectAdapter("ClassDemo");
    com::jd::CommontIceRpcServicePtr classdemo = new com::jd::CommontIceRpcServiceI;
//	communicator()->getProperties()->setProperty("ClassDemo.ThreadPool.Size", "100");
	communicator()->getProperties()->setProperty("Ice.ThreadPool.Server.ThreadPriority", "10");
	communicator()->getProperties()->setProperty("ClassDemo.ThreadPool.ThreadPriority", "50");

    adapter->add(classdemo, communicator()->stringToIdentity("classdemo"));
    adapter->activate();
    communicator()->waitForShutdown();
    return EXIT_SUCCESS;
}

