
**中文简介**

KNV是一个模式自由的高性能树型协议处理引擎，是对Key-Value的一个通用结构化扩展。

**应用特性**

正如NoSQL对SQL的彻底变革一样，KNV是对Key-Value的彻底变革！
大家都很熟悉Key-Value系统，但KV操作极不方便：
-- 你不得不自己做乐观锁（读返回seq->修改->带seq写->seq变化则重头开始）；
-- 你不得不拉取一堆数据，然后再提取你想要的部分；
-- 为了修改一个bit，你不得不把一串数据拉取修改后再一起提交更新
不过现在有KNV，一切都解决了。KNV时代来了，你还在犹豫吗？
应用场景

KNV是专门针对以Protocol Buffers（或类似树型结构）存储的存储系统开发的，有3个应用场景：

    存储系统 处理用户请求的通用解决方法（Get/Set/Delete），可以适配任意用户自定义的数据结构。

    代理/通用逻辑层 服务器处理Protocol Buffers协议的通用方法 – 比如通用Proxy，透传多个协议， 对请求包进行鉴权、限频、统计等等操作，还支持对包体内容进行审计，使用KNV协议， 这种方法就有可能而且很简单。

    原生态 Protocol Buffers打解包库 的高性能代替品。

**设计原理**
设计原理的说明文档：Key-N-Value--基于Protocol Buffers的树型协议处理引擎（https://blog.csdn.net/shaneyuee/article/details/136992263）
-----


 
 What is KNV?
------------------------------------------
 KNV(Key-N-Value) is a source code library for manipulating protocol data without knowing the detail of it contents. KNV can be seen as a Key-Value extension that supports all data structures.
 
 As we know, Key-Value systems such as memcached is very inconvenient, for examples:
 
 ---- If you wish to read a sub-field in value, you need to read the whole value and extract it by your self;

 ---- If data can be modified by different clients, they have to avoid write-conflict by themselves, e.g., by undertaking an optimistic locking;

 ---- If you want to increase a field in value, you still need to read and write all value, consuming a lot of network resources.

 To solve these problems, Redis comes to help by supporting much more complicated data structures. But redis still has many difficulties:
 
 ---- Each data structure has its own implementation, making it diffcult to add more data structures, thus the end-users have to FIT their data model to existing data structures;

 ---- Once a data structure is used, it is almost impossible to change to another one;

 ---- ... ...

 OK, here comes KNV, all the above difficulties are solved completely.
 
 
  Why KNV?
------------------------------------------
 KNV serves for 3 main purposes:
 
   1, As a fast protocol engine, supporting 1M+ processes per second. As KNV supports only Protocol Buffers for now, it can be use to handle PB network data in place of Google's original PB library.

   2, As a schema-free protocol inspecter/modifier for general-purpose network server. For example, a network proxy can use KNV to examine network data and modify their contents if necessary.

   3, As a protocol and data storage engine for general data storage server.


 Request Tree
------------------------------------------
 A network invocation can be treated as an operation performed on a set of objects (or sub-objects), thus we can denote the invocation as:
 
         Network invocation = (Operation, Data, Path)

 For example, I have all my classmates' information stored on a remote server, and I wish to change my classmate A's remark to Alice and B's to Bob, my invocation will be like this:
          
         Operation = Set value
         
         Data = {Alice, Bob}
         
         Path = I->Classmates->{A,B}->Remark
         
 Using traditional method, the protocol designer needs to take classmate and remark into account, but if you are implementing a general sotrage server, you probably do not like this.
 
 Many known storage servers use Key-Value (Memcached) or complicated data structures (Redis) to implement a generalized method, but still have many tradeoffs as mentioned above.
 
 KNV solves these problems by introducing the concept of "Request Tree".
 
 A request tree is the directories of all requested sub-objects linked together. Given a request tree, the storage server knowns exactly what sub-objects are to be operated on, whitout knowing the meaning of them.


 Key-N-Value philosophy:
------------------------------------------
  A KNV tree is common PB tree, except that each node is identified by Tag + Key.
  Tag is the PB tag number, Key is the value of a special sub node with tag=1.
  A Leaf is a special node that is either non-expandable or has not been expanded.

  The first 10 tag numbers are reserved for META data in a node,
  if you wish KNV tree to manage nodes, please define your data nodes beginning at tag 11.


Modification history
------------------------------------------
         2013-10-12   Yu Zhenshen       Created
         
         2013-11-04   Yu Zhenshen       Add parent pointer and eval_size to optimize folding
         
         2014-01-17   Yu Zhenshen       Use mem_pool for dynamic memory management
         
         2014-01-28   Yu Zhenshen       Use KnvHt to optimize hash initialization
         
         2014-05-17   Yu Zhenshen       Meta use KnvNode instead of KnvLeaf


Example usage for PB encoding/decoding:
------------------------------------------

Step 1, write your own .proto file:

--------test.proto-----------------

         message TestRequestBody
         {
         
         	optional string user_id = 1;
         	
         	optional uint32 operation = 11;
         	
         	optional string reason = 12;
         	
         };
         
         message TestResponseBody
         {
         
         	optional string result = 11;
         	
         };
         
---------end of test.proto-----------

Step 2, use knvshow to generate a corresponding .h file:

Execute:
         ./knvshow -p test.proto > test.knv.h

--------test.knv.h-------------------
         
         enum TestRequestBodyTags
         {
         
         	TestRequestBodyTag_user_id = 1,
         	
         	TestRequestBodyTag_operation = 11,
         	
         	TestRequestBodyTag_reason = 12,
         };
          
         enum TestResponseBodyTags
         
         {
         	TestResponseBodyTag_result = 11,
         	
         };
         
-------end of test.knv.h-------------


Step 3, write your own code that handles protocol with knv API:

         KnvNode *req = KnvNode::NewTree(1); // Eache node requires a tag for KNV, but each PB message needs not if it is a root node, so here we assign one anyway
         
         if(req==NULL) // something goes wrong
         
         {
         
         	cout << "KnvNode::NewTree failed: " << KnvNode::GetGlobalErrorMsg() << endl;
         	
         	return -1;
         	
         }
         
         req->SetFieldStr(TestRequestBodyTag_user_id, 4, "test");
         
         req->SetFieldInt(TestRequestBodyTag_operation, 1);
         
         req->SetFieldStr(TestRequestBodyTag_reason, 8, "for test");
         
         char buffer[1024];
         
         int len = sizeof(buffer);
         
         req->Serialize(buffer, &len, false); // not encoding header tag
         
         KnvNode::Delete(req); // you should delete req when it is no longer in use
         
         // Now buffer contains the same data as TestRequestBody.SerialzeToString()
         
         // Next, write your own code that handles remote procedure call, we assume that buffer contains the whole response packet
         
         string rsp_body(buffer, len);
         
         KnvNode *rsp = KnvNode::NewFromMessage(rsp_body);
         
         cout << rsp->GetFieldStr(TestResponseBodyTag_result) << endl;
         
         KnvNode::Delete(rsp); // you should delete rsp when it is no longer in use
