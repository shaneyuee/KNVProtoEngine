/*
Tencent is pleased to support the open source community by making Key-N-Value Protocol Engine available.
Copyright (C) 2015 THL A29 Limited, a Tencent company. All rights reserved.
Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with the License. You may obtain a copy of the License at
http://www.apache.org/licenses/LICENSE-2.0
Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.
*/

#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <fstream>
#include <iostream>

#include "knv_node.h"
#include "pb3_test.pb.h"

int main(int argc, char *argv[])
{
	pb3test::MsgBody body;
	pb3test::Person &p = *body.mutable_person();
	p.set_name("shaneyu");
	int delta = 1;
	for(int i=0; i<10; i++)
	{
		delta = -delta;
		p.add_ids(i);
		p.add_schools(i+((float)i)/10);
		p.add_interests(i+((double)i)/1000);
		p.add_friendids(i*delta);
	}
	std::string s;
	body.SerializeToString(&s);
  
	std::string outfile("./pb3_test.out");
	ofstream fs(outfile.c_str(), ios_base::binary|ios_base::out|ios_base::trunc);
	fs<<s;
	fs.close();

	KnvNode *node = KnvNode::New(s);
	if(node==NULL)
	{
		fprintf(stderr, "KnvNode::NewFromMessage() failed: %s\n", KnvNode::GetGlobalErrorMsg());
		exit(-1);
	}

	node->Print("balabala:");

	int ret;
	vector<uint64_t> ids;
	vector<float> schools;
	vector<double> interests;
	vector<int64_t> friendids;
	if((ret=node->GetFieldsInt(2, ids))!=10)
	{
		fprintf(stderr, "KnvNode::GetFieldsInt() returned unexpected fields num %d\n", ret);
		exit(-1);
	}
	if((ret=node->GetFieldsFloat(3, schools))!=10)
	{
		fprintf(stderr, "KnvNode::GetFieldsFloat() returned unexpected fields num %d\n", ret);
		exit(-1);
	}
	if((ret=node->GetFieldsDouble(4, interests))!=10)
	{
		fprintf(stderr, "KnvNode::GetFieldsDouble() returned unexpected fields num %d\n", ret);
		exit(-1);
	}
	if((ret=node->GetFieldsSInt(100, friendids))!=10)
	{
		fprintf(stderr, "KnvNode::GetFieldsSInt() returned unexpected fields num %d\n", ret);
		exit(-1);
	}

	printf("name:%s\n", node->GetFieldStr(1).c_str());
	printf("ids:%lu", ids[0]);
	for(int i=1; i<ids.size(); i++)
		printf(",%lu", ids[i]);
	printf("\nfriendids:%ld", friendids[0]);
	for(int i=1; i<friendids.size(); i++)
		printf(",%ld", friendids[i]);
	printf("\nschools:%f", schools[0]);
	for(int i=1; i<schools.size(); i++)
		printf(",%f", schools[i]);
	printf("\ninterests:%g", interests[0]);
	for(int i=1; i<interests.size(); i++)
		printf(",%g", interests[i]);
	printf("\n");

	return 0;
}
