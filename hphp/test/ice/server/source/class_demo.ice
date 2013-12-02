module com{
	module jd{
		class A{
			string id;
			int num;
		};
		class B{
			string id;
			string name;
		};
		class ClassDemo{
			string id;
			int idlen;
			A classA;
			B classB;
		};
		sequence<ClassDemo> ClassDemoS;
		sequence<long> LongList;
		//sequence<string> StringList;
		//sequence<int> IntList;

		exception RpcException {
				int resultCode;
				string msg;
		};

		
		interface CommontIceRpcService{
			ClassDemo getClassDemo();
			ClassDemo newClassDemo(ClassDemo cd);
			ClassDemoS getClassDemoS();
			void setClassDemos(ClassDemoS cds);
			LongList getLongList();
		//	StringList getStringList();
		//	IntList getIntList();

			LongList setLongList(LongList ll);
		//	StringList setStringList(StringList ll);
		//	IntList setIntList(IntList ll);
			
		//	string getString();
		//	int getInt();
		//	long getLong();
		
		//	void setMulP(int a,string b,long c);
		//	void setMulClass(ClassDemo a,ClassDemo b,int c);
			
			int  getOtherCommentById() throws RpcException;

		};
	};
};
