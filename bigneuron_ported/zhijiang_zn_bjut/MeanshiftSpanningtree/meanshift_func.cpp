/* example_func.cpp
* This file contains the functions used in plugin domenu and dufunc, you can use it as a demo.
* 2012-02-13 : by Yinan Wan
*/

#include <v3d_interface.h>
#include "v3d_message.h"
#include "stackutil.h"
#include "meanshift_func.h"
#include "../../../released_plugins/v3d_plugins/neurontracing_vn2/app2/heap.h"
#include "../../../../v3d_external/v3d_main/neuron_editing/v_neuronswc.h"
#include "../../../released_plugins/v3d_plugins/sort_neuron_swc/sort_swc.h"
#include <vector>
#include <iostream>

using namespace std;

/* function used in DOMENU typically takes 2 inputs:
* "callback" - provide information from the plugin interface, and 
* "parent"   - the parent widget of the Vaa3D main window
*/


#define GET_IND(x,y,z) (x) + (y) * sz_x + (z) * sz_x* sz_y
#define GET_X(ind) ind % sz_x
#define GET_Y(ind) (ind / sz_x) % sz_y
#define GET_Z(ind) (ind / (sz_x*sz_y)) % sz_z
#define ABS(a) (a) > 0 ? (a) : -(a)
#define getParent(n,nt) ((nt).listNeuron.at(n).pn<0)?(1000000000):((nt).hashNeuron.value((nt).listNeuron.at(n).pn))

int *flag;//���ڱ��ÿ�����ص��Ƿ�MeanShift����

QMap<int,QList<Node*> > Map_finalnode_list;//��ֵ�����ӵ�����QList�Ƕ�Ӧ���ӵ����Ľڵ���
QMap<int,Node*> Map_rootnode;//int�ǽڵ�ı�ţ�Ҳ�����ı��
QMultiMap<int,Node*> Map_allnodes;//��ͼ���е�ÿһ�����ص㶼����mark
QMap<int,QList<Node*> > finalclass_node;//���յ������ط�����
QList<QList<NeuronSWC> > result_tree_part;
QList<NeuronSWC> result_final_tree;

QMap<V3DLONG,int> Map_finalnode; 
QMap<V3DLONG,int> Map_nodes;//��ֵ������ά���꣬int��ʾmark������������е�ķ���
QMap<V3DLONG,V3DLONG> Map_allnode;//��Map_allnodes������һ�������Ǹ����еĵ���з��࣬��ͬ���Ǳ�����ǲ�����Ķ�ά����
QList <NeuronSWC> result_list;
vector<QList<Node*> > v_List;//���ڱ���ÿ�����ص��ҵ���·��
QMap<V3DLONG,QList<Node*> > root_class;//���ڵ��ά�����Ӧ�͸��ڵ������ͬ�ĵ�



Node getnode(Node *node)
{
	Node result;
	result.x=node->x;
	result.y=node->y;
	result.z=node->z;
	return result;


}

bool contain(QList<Node> *queue,V3DLONG x,V3DLONG y,V3DLONG z)
{
	bool flag=false;
	for(int i=0;i<queue->size();i++)
	{
		//Node* temp=new Node();
		if((queue->at(i).x==x)&&(queue->at(i).y==y)&&(queue->at(i).z==z))
		{
			flag=true;
			break;

		}
	} 

	return flag;

}
void enlarge_radiusof_single_node_xy(unsigned char * &img1d,Node * &node,V3DLONG sz_x, V3DLONG sz_y, V3DLONG sz_z)
{
	//����xyƽ��4����İ뾶����

	//����xyƽ��4����İ뾶����
	//queue=new QQueue<Node*>();
	QList<Node> *list_node=new QList<Node>();
	int allnodes=0;
	int back_nodes=0;
	int max=0;//�����뾶
	double threshold=30;

	//queue->enqueue(node);
	//queue->append(node);
	//list.append(GET_IND(node->x,node->y,node->z));
	list_node->append(getnode(node));
	

	while(1)//���ڸ��ڵ���1Ϊ������һ��һ�����������ѷ�Χ�ڵĵ㶼�ӵ���������
	{
		
		//Node* head=queue->head();
		//Node* head=queue->first();
		//V3DLONG head=list.begin();
		
		Node head=list_node->first();
		Node* up_queue=new Node(head.x,head.y-1,head.z);
		Node* down_queue=new Node(head.x,head.y+1,head.z);
		Node* left_queue=new Node(head.x-1,head.y,head.z);
		Node* right_queue=new Node(head.x+1,head.y,head.z);
		/*Node *up_queue=up;
		Node *down_queue=down;
		Node *left_queue=left;
		Node *right_queue=right;*/
		
		//if(!queue->contains(up_queue))//��������û�а����ýڵ�
		if(!contain(list_node,up_queue->x,up_queue->y,up_queue->z)) 
		//if(!queue->contains(GET_IND(up->x,up->y,up->z)))
		{
			allnodes=allnodes+1;
			//printf("%lf\n",(double)img1d[GET_IND(up->x,up->y,up->z)]);
			//printf("%ld   %ld   %ld\n",up->x,up->y,up->z);
			if(img1d[GET_IND(up_queue->x,up_queue->y,up_queue->z)]<=30)
			{
				back_nodes=back_nodes+1;
				//printf("back:::%d\n",back_nodes);
			}
			//list.append(GET_IND(up->x,up->y,up->z));
			//queue->enqueue(up_queue);
			//queue->append(up_queue);
			list_node->append(getnode(up_queue));
			//delete up;

		}

		//if(!queue->contains(down_queue))
		if(!contain(list_node,down_queue->x,down_queue->y,down_queue->z))
		//if(!queue->contains(GET_IND(down->x,down->y,down->z)))
		{
			//printf("no down\n");
			allnodes=allnodes+1;
			//printf("%lf\n",(double)img1d[GET_IND(down->x,down->y,down->z)]);
			//printf("%ld   %ld   %ld\n",down->x,down->y,down->z);
			if(img1d[GET_IND(down_queue->x,down_queue->y,down_queue->z)]<=30)
			{
				back_nodes=back_nodes+1;
				//printf("back:::%d\n",back_nodes);
			}
			//list.append(GET_IND(down->x,down->y,down->z));
			//queue->enqueue(down_queue);
			//queue->append(down_queue);
			list_node->append(getnode(down_queue));
			//delete down;

		}

		//if(!queue->contains(left_queue))
		//if(!queue->contains(GET_IND(left->x,left->y,left->z)))
		if(!contain(list_node,left_queue->x,left_queue->y,left_queue->z))
		{
			//printf("no left\n");
			allnodes=allnodes+1;
			//printf("%lf\n",(double)img1d[GET_IND(left->x,left->y,left->z)]);
			//printf("%ld   %ld   %ld\n",left->x,left->y,left->z);
			if(img1d[GET_IND(left_queue->x,left_queue->y,left_queue->z)]<=30)
			{
				back_nodes=back_nodes+1;
				//printf("back:::%d\n",back_nodes);
			}
			//list.append(GET_IND(left->x,left->y,left->z));
			//queue->enqueue(left_queue);
			//queue->append(left_queue);
			list_node->append(getnode(left_queue));
			//delete left;

		}

		//if(!queue->contains(right_queue))
		if(!contain(list_node,right_queue->x,right_queue->y,right_queue->z))
		//if(!queue->contains(GET_IND(right->x,right->y,right->z)))
		{
			//printf("no right\n");
			allnodes=allnodes+1;
			//printf("%lf\n",(double)img1d[GET_IND(right->x,right->y,right->z)]);
			//printf("%ld   %ld   %ld\n",right->x,right->y,right->z);
			if(img1d[GET_IND(right_queue->x,right_queue->y,right_queue->z)]<=30)
			{
				back_nodes=back_nodes+1;
				//printf("back:::%d\n",back_nodes);
			}
			//list.append(GET_IND(right->x,right->y,right->z));
			//queue->enqueue(right_queue);
			//queue->append(right_queue);
			list_node->append(getnode(right_queue));
			//queue->
			//delete right;

		}
		//printf("size:::%d   \n",queue.size());
		//printf("%d   %d\n",allnodes,back_nodes);

		double up_r=(double)sqrt((double)(node->x-up_queue->x)*(node->x-up_queue->x)+(double)(node->y-up_queue->y)*(node->y-up_queue->y)+(double)(node->z-up_queue->z)*(node->z-up_queue->z));
		double down_r=(double)sqrt((double)(node->x-down_queue->x)*(node->x-down_queue->x)+(double)(node->y-down_queue->y)*(node->y-down_queue->y)+(double)(node->z-down_queue->z)*(node->z-down_queue->z));
		double left_r=(double)sqrt((double)(node->x-left_queue->x)*(node->x-left_queue->x)+(double)(node->y-left_queue->y)*(node->y-left_queue->y)+(double)(node->z-left_queue->z)*(node->z-left_queue->z));
		double right_r=(double)sqrt((double)(node->x-right_queue->x)*(node->x-right_queue->x)+(double)(node->y-right_queue->y)*(node->y-right_queue->y)+(double)(node->z-right_queue->z)*(node->z-right_queue->z));
		float per=(float)back_nodes/(float)allnodes;
		
		
		if(per>0.05)
		{

			node->r=(up_r+down_r+left_r+right_r)/4;	
			//printf(" radius::%lf\n",node->r);
			delete up_queue;
		delete down_queue;
		delete left_queue;
		delete right_queue;

			
			break;
		}else{
		//����
			//queue->pop_front();
			list_node->pop_front();
			//list.detach();
			//list.pop_front();
		delete up_queue;
		delete down_queue;
		delete left_queue;
		delete right_queue;
		}
	}
	//queue->clear();
	//qDeleteAll(list_node);
	list_node->clear();
	delete list_node;
	/*for(int i=0;i<queue->size();i++)
	{
		Node *temp=queue->at(i);
		
		//delete (temp);
		//printf("%ld\n",queue->at(i)->x);
		//printf("temp::%ld\n",temp->x);


	}*/

}


void writeSWC_file(char* path,QList<NeuronSWC> listNeuron)
{
	FILE * fp = fopen(path, "a");
	if (!fp) return;

	fprintf(fp, "#name\n");
	fprintf(fp, "#comment\n");
	fprintf(fp, "##n,type,x,y,z,radius,parent\n");
	for(int i=0;i<listNeuron.size();i++)
	{
		NeuronSWC temp=listNeuron.at(i);
		fprintf(fp, "%ld %d %f %f %f %f %ld\n",
			temp.n, temp.type,  temp.x,  temp.y,  temp.z, temp.r, temp.parent);

	}
	fclose(fp);
	return;

}



void printSWCByQList_QList(QList<QList <NeuronSWC> > result_list,char* path)
{
	FILE * fp = fopen(path, "a");
	if (!fp) return;

	fprintf(fp, "#name\n");
	fprintf(fp, "#comment\n");
	fprintf(fp, "##n,type,x,y,z,radius,parent\n");
	for(int i=0;i<result_list.size();i++)
	{
		QList<NeuronSWC> list1=result_list.at(i); 
		for(int j=0;j<list1.size();j++)
		{
			NeuronSWC temp2=list1.at(j);
			fprintf(fp, "%ld %d %f %f %f %f %ld\n",
				temp2.n, temp2.type,  temp2.x,  temp2.y,  temp2.z, temp2.r, temp2.parent);
		}
	}
	fclose(fp);
	return;
}

void printSWCByQList_Neuron(QList <NeuronSWC> result_list,char* path)
{
	V3DLONG number=0;
	FILE * fp = fopen(path, "a");
	if (!fp) return; 

	fprintf(fp, "#name\n");
	fprintf(fp, "#comment\n");
	fprintf(fp, "##n,type,x,y,z,radius,parent\n");

	//for(int i=0;i<result_list.size();i++)
	{
		//QList<NeuronSWC> temp1=result_list.at(i);
		for(int j=0;j< result_list.size();j++)
		{
			NeuronSWC temp2= result_list.at(j);
			fprintf(fp, "%ld %d %f %f %f %f %ld\n",
				temp2.n, temp2.type,  temp2.x,  temp2.y,  temp2.z, temp2.r, temp2.parent);

		}
	}

	fclose(fp);
	return;


}
void printSWCByVector_List(vector<QList<Node*> > vector_List,char* path)
{
	V3DLONG number=0;
	FILE * fp = fopen(path, "a");
	if (!fp) return;

	fprintf(fp, "#name\n");
	fprintf(fp, "#comment\n");
	fprintf(fp, "##n,type,x,y,z,radius,parent\n");

	for(int i=0;i<vector_List.size();i++)
	{
		QList<Node*> temp=vector_List.at(i);
		for(int j=0;j<temp.size();j++)
		{
			Node* temp_node=temp.at(j);
			fprintf(fp, "%ld %d %ld %ld %ld %5.3f %ld\n",
				number, 1,  temp_node->x,  temp_node->y,  temp_node->z, temp_node->r, -1);
			number++;
		}

	}
	fclose(fp);
	return;


}

void printSwcByList(QList<Node*> nodeList,char* path)
{
	V3DLONG number=0;
	FILE * fp = fopen(path, "a");
	if (!fp) return;

	fprintf(fp, "#name\n");
	fprintf(fp, "#comment\n");
	fprintf(fp, "##n,type,x,y,z,radius,parent\n");

	for(int i = 0; i < nodeList.size(); i++)
	{
		Node* elem = nodeList.at(i);
		fprintf(fp, "%ld %d %ld %ld %ld %5.3f %ld\n",
			number, 1,  elem->x,  elem->y,  elem->z, elem->r, -1);
		number++;
	}

	fclose(fp);

	return;
}
void printSwcByMap(QMap<int,Node*> nodeMap,char* path)
{
	V3DLONG number=0;
	FILE * fp = fopen(path, "wt");
	if (!fp) return;

	fprintf(fp, "#name\n");
	fprintf(fp, "#comment\n");
	fprintf(fp, "##n,type,x,y,z,radius,parent\n");


	for(QMap<int,Node*>::iterator iter = nodeMap.begin(); iter != nodeMap.end(); iter++)
	{
		Node* temp_node=iter.value();;
		fprintf(fp, "%ld %d %ld %ld %ld %5.3f %ld\n",
			number, 1,  temp_node->x,  temp_node->y,  temp_node->z, temp_node->r, -1);
		number++;
	}


	fclose(fp);

	return;
}

void printSWCByMultiMap(QMultiMap<int,int> multimap,char *filename)
{
	FILE * fp = fopen(filename, "a");//׷��
	if (!fp) return;
	V3DLONG number=0;
	int front=0;
	int next=0;

	fprintf(fp, "#name\n");
	fprintf(fp, "#comment\n");
	fprintf(fp, "##n,type,x,y,z,radius,parent\n");

	for(QMultiMap<int,int>::iterator iter2 =multimap.begin(); iter2 != multimap.end();  iter2++)
	{
		next=iter2.value();
		if(front==next)
		{
			continue;

		}else
		{
			fprintf(fp, "%ld %d %ld %ld %ld %5.3f %ld\n",
				number, 1,  Map_rootnode[next]->x,  Map_rootnode[next]->y,  Map_rootnode[next]->z, Map_rootnode[next]->r, -1);
			number++;


		}

		front=next;


	}


}

void printSWCByMap_ListInt(QMap<int,QList<int> >  List,char * filename)
{
	FILE * fp = fopen(filename, "a");//׷��
	if (!fp) return;
	V3DLONG number=0;

	fprintf(fp, "#name\n");
	fprintf(fp, "#comment\n");
	fprintf(fp, "##n,type,x,y,z,radius,parent\n");
	//printf("11111111111111111111111111111111111111111111111\n");

    for(QMap<int,QList<int> >::iterator iter2 =List.begin(); iter2 != List.end();  iter2++)
	{
		QList<int> List2=iter2.value();
		//printf("size:::%d\n",List2.size());
		for(int i=0;i<List2.size();i++)
		{
			int elem = List2.at(i);
			//printf("elem:::%d\n",elem);
			fprintf(fp, "%ld %d %ld %ld %ld %5.3f %ld\n",
				number, 1,  Map_rootnode[elem]->x,  Map_rootnode[elem]->y,  Map_rootnode[elem]->z, Map_rootnode[elem]->r, -1);
			number++;

		}

	}
	fclose(fp);

	return;

}

void printSWCByMap_List(QMap<int,QList<Node*> >  List,char * filename)
{//��������ǽ�QMap<V3DLONG,QList<Node*>>����д��SWC�ļ���
	FILE * fp = fopen(filename, "a");//׷��
	if (!fp) return;
	V3DLONG number=0;

	fprintf(fp, "#name\n");
	fprintf(fp, "#comment\n");
	fprintf(fp, "##n,type,x,y,z,radius,parent\n");


    for(QMap<int,QList<Node*> >::iterator iter2 =List.begin(); iter2 != List.end();  iter2++)
	{
		QList<Node*> List2=iter2.value();
		for(int i=0;i<List2.size();i++)
		{
			Node* elem = List2.at(i);
			fprintf(fp, "%ld %d %ld %ld %ld %5.3f %ld\n",
				number, iter2.key(),  elem->x,  elem->y,  elem->z, elem->r, -1);
			number++;

		}
	}

	fclose(fp);

	return;
}


int meanshift_plugin_vn4(V3DPluginCallback2 &callback, QWidget *parent)
{
	v3dhandle curwin = callback.currentImageWindow();
	if(!curwin)
	{
		//QMessageBox::information(0, title, QObject::tr("No image is open."));
		return -1;
	}
	Image4DSimple *p4d = callback.getImage(curwin);
	V3DLONG sz_x = p4d->getXDim();
	V3DLONG sz_y = p4d->getYDim();
	V3DLONG sz_z = p4d->getZDim();
	unsigned char* img1d = p4d->getRawDataAtChannel(1);
	flag=new int [sz_x*sz_y*sz_z];

	V3DLONG r=10;
	V3DLONG count=0;
	int times=100;
	int num_mark=1;//�����ڳ�ʼ����ʱ������ӵ���з�����
	
	printf("### Initializing...  ###\n");//20150315��һ��ʼ�Ͷ�����ͼ����г�ʼ��������Ѱ�Ҳ�������û�б�Ҫ�ģ�����һ��Ѱ����ֹ�㣬һ�߸������Ľڵ���б�ǣ��Ժ������޸�
	

	for(V3DLONG ii=0;ii<sz_x;ii++)
	{
		for(V3DLONG jj=0;jj<sz_y;jj++)
		{
			for(V3DLONG kk=0;kk<sz_z;kk++)
			{
				V3DLONG ind=GET_IND(ii,jj,kk);//����ά����ת���ɶ�ά���꣬ͨ����ά����������ǿ��
				
				if(img1d[ind]==0)//����ǿ��Ϊ0������㲻�迼�ǣ�ֻ�Ծ�������ǿ�ȵĵ����meanshift
					continue;
				/*	if(img1d[ind]<30)//����ǿ�ȳ���ĳһ��ֵ������㲻�迼�ǣ�ֻ�Ծ�������ǿ�ȵĵ����meanshift
				continue;*/
				if(!found_final(img1d,ii,jj,kk,sz_x, sz_y, sz_z, r))//û�ж�
				{
					Node* final=new Node(ii,jj,kk);
					
					final->class_mark=num_mark;
					final->parent=-1;//���ڵ���Ϊ-1
					final->number=0;//ÿ�����ڵ����ű��Ϊ0
					Map_finalnode.insert(ind,num_mark);//�����ڵ���������,ind�Ƕ�ά����,num_mark�Ǳ��
					enlarge_radiusof_single_node_xy(img1d,final,sz_x, sz_y, sz_z);//������ڵ�İ뾶
					Map_rootnode.insert(num_mark,final);//�����ڵ��������ţ�num_mark�����final��Ӧ��Ӧ�Ľڵ�
					num_mark++;

				}

			}
		}
	}

	printf("###   initializing finished   ###\n");
	printf("###  start meanshift   ###\n");
	for(V3DLONG i=0;i<sz_x;i++)
	{
		for(V3DLONG j=0;j<sz_y;j++)
		{
			for(V3DLONG k=0;k<sz_z;k++)
			{
				V3DLONG ind=GET_IND(i,j,k);//����ά����ת���ɶ�ά���꣬ͨ����ά����������ǿ��
				if(img1d[ind]==0)//����ǿ��Ϊ0������㲻�迼�ǣ�ֻ�Ծ�������ǿ�ȵĵ����meanshift
					continue;
				/*if(img1d[ind]<30)//����ǿ�ȳ���ĳһ��ֵ������㲻�迼�ǣ�ֻ�Ծ�������ǿ�ȵĵ����meanshift
				continue;*/
				if(flag[ind]==1)

				{
					
					continue;//�������������ص㲻�ٱ�����

				}
				meanshift_vn4(img1d,i,j,k,sz_x,sz_y,sz_z,r,times);//ͨ��������������еĽڵ㶼�����ű���ķŵ���Map_finalnode_list<int,QList<Node*>>��
			}

		}

	}

	merge_rootnode(Map_rootnode,img1d,sz_x,sz_y,sz_z);//�����������������һ���µ�Map_rootnode,��֮ǰ�ĸ��ڵ����٣�����������ڵ�֮�䲻���غ�

	construct_tree(finalclass_node, sz_x, sz_y, sz_z);//�õ������е���������
	printSWCByQList_Neuron(result_list,"D:\\result\\meanshift_result.swc");
	//writeSWC_file("D:\\result\\meanshift_result_tree_part.swc",result_final_tree);
	delete []flag;
}



void merge_rootnode(QMap<int,Node*> &rootnodes,unsigned char * &img1d,V3DLONG sz_x,V3DLONG sz_y,V3DLONG sz_z)//�������Ӧ��֪����Щ���ڵ㱻�ϲ�������Щ���ڵ���
{//��ÿһ�����ڵ㶼��һ��covering list,���covering list������������ڵ㶼��������Щ�����ڵ�
	
	QMultiMap<double, int> r_root;//double����
	QMap<int,Node*> root;//�������rootnodes���в���
	QMultiMap<int,int> parent_root;//���ڸ�ÿ�����ڵ�Ͱ������ĸ��ڵ㽨����ϵ����1��V3DLONG������2��

	//��Ϊһ�����ڵ��п��ܱ�����ڵ�������
	QMap<int,int> evidence2;
	printf("rootnodes:::%d\n",rootnodes.size());
	for(QMap<int,Node*>::iterator iter =rootnodes.begin(); iter != rootnodes.end(); iter++)
	{
		Node* elem=iter.value();
		int key=iter.key();
		r_root.insert(elem->r,key);
		evidence2.insert(key,0);
		parent_root.insert(key,key);//ÿһ�����ڵ㶼Ҫ���ൽ�Լ��������

		for(QMap<int,Node*>::iterator iter1 =rootnodes.begin(); iter1 != rootnodes.end(); iter1++)
		{
			Node* elem1=iter1.value();
			int key1=iter1.key();
			if((elem1->x==elem->x)&&(elem1->y==elem->y)&&(elem1->z==elem->z))
				continue;//�Լ������Լ����
			double dis=(double)sqrt((double)(elem->x-elem1->x)*(elem->x-elem1->x)+(double)(elem->y-elem1->y)*(elem->y-elem1->y)+(double)(elem->z-elem1->z)*(elem->z-elem1->z));

			if(dis/(elem->r+elem1->r)<1.5)
			{//��ǰ�ڵ��Ѿ���ǰ��Ľڵ�����������ǰ�ڵ㲻Ӧ�ð���ǰ��Ľڵ�
				parent_root.insert(key,key1);//parent_root����Ӧ���ǰ��������еĸ��ڵ㣬��Map_rootnodeһ��
				//parent_root_versus.insert(key1,key);
			}
		}

		if(!parent_root.contains(key))
		{
			parent_root.insert(key,key);//�п���ĳЩ���ڵ�İ뾶��Χ�ڲ������κθ��ڵ㣬��ֻ�����Լ�

		}
	}

    QMap<int,QList<int> > Map_finallist_new;//�������еĸ��ڵ��Լ����������ص�
	//��������Ҫ��һ�����������������еĵ㶼���ൽ�ϲ�����ֹ����
	printf("parent_root:::%d\n",parent_root.size());
	printf("r_root:::%d\n",r_root.size());

	//Ӧ��дһ��ѭ�������뾶�Ӵ�С�ţ�ɸѡ�����յĸ��ڵ�
	for(QMultiMap<double,int>::iterator iter3=r_root.end();iter3!=r_root.begin();iter3--)
	{//������һ�����⣬����r_rootû�м��㣬����һ����Ӧ��Ӱ�첻���Ժ���
		int times=0;
		int root_key=iter3.value();
		if(iter3==r_root.end())
		{

			continue;

		}
		if(evidence2[root_key]==1)
		{

			continue;//˵���Ѿ������˰�����ȥ�ˣ�������Ϊ���յĸ��ڵ�

		}else
		{
			root.insert(root_key,Map_rootnode[root_key]);
			QMultiMap<int,int>::iterator begin=parent_root.lowerBound(root_key);
			QMultiMap<int,int>::iterator end=parent_root.upperBound(root_key);
			while(begin!=end)
			{

				Map_finallist_new[root_key].append(begin.value());
				evidence2[begin.value()]=1;
				begin++;
				times++;//for testing
			}
			//printf("times:::%d\n",times);
		}

	}


	//20150311,����Map_finallist_new���������ص���й��࣬Map_finallist_newӦ���Ѿ���������һ����2��������
    for(QMap<int,QList<int> >::iterator iter3 =Map_finallist_new.begin(); iter3!=Map_finallist_new.end(); iter3++)
	{
		int first_index=iter3.key();

		for(int i=0;i<Map_finallist_new[first_index].size();i++)
		{
			int second_index=Map_finallist_new[first_index].at(i);

			for(int j=0;j<Map_finalnode_list[second_index].size();j++)
			{
				Node* elem3=Map_finalnode_list[second_index].at(j);
				if(Map_nodes[GET_IND(elem3->x,elem3->y,elem3->z)]!=-1)//����finalclass_node[first_index]�����ǲ����Ѿ������˽�Ҫ�����ĵ�
				{
					finalclass_node[first_index].append(Map_finalnode_list[second_index].at(j));
				}
				Map_nodes[GET_IND(elem3->x,elem3->y,elem3->z)]=-1;
			}
		}
	}
	
	printSwcByMap(root,"D:\\result\\finalroot.swc");


}

void construct_tree(QMap<int,QList<Node*> > finalclass_node,V3DLONG sz_x,V3DLONG sz_y,V3DLONG sz_z)
{
    for(QMap<int,QList<Node*> >::iterator iter=finalclass_node.begin();iter!=finalclass_node.end();iter++)
	{
		{
			if(iter.value().size()<=1)
				continue;

			QList<Node*> seed=iter.value();
			//��ÿ���������ص㰴�հ뾶��С����ɾ����������ÿ�����������	
			QList<Node*> seeds=trim_nodes(seed, sz_x, sz_y, sz_z);
			
			V3DLONG marknum = seeds.size();

			double** markEdge = new double*[marknum];
			for(int i = 0; i < marknum; i++)
			{
				markEdge[i] = new double[marknum];
				//fprintf(debug_fp,"markEdge[i]:%lf\n",markEdge[i]);
			}

			double x1,y1,z1;
			for (int i=0;i<marknum;i++)
			{
				x1 = seeds.at(i)->x;
				y1 = seeds.at(i)->y;
				z1 = seeds.at(i)->z;
				for (int j=0;j<marknum;j++)
				{
					//������Ȩֵ�ķ��������⣬�������Ǿ�������صĻ��־��ò����ã����Կ��ǽṹ����
					markEdge[i][j] = sqrt(double(x1-seeds.at(j)->x)*double(x1-seeds.at(j)->x) + double(y1-seeds.at(j)->y)*double(y1-seeds.at(j)->y) + double(z1-seeds.at(j)->z)*double(z1-seeds.at(j)->z));
					//fprintf(debug_fp,"markEdge[i][j]:%lf\n",markEdge[i][j]);
				}
			}//����Ǽ������ӵ�֮��ľ��룬Ӧ�����ڹ���ߣ�������ߵĳ���

			//NeutronTree structure
			NeuronTree marker_MST;
			QList <NeuronSWC> listNeuron;//NeuronSWCӦ���ǽڵ����˼������ڵ㹹��һ����
			QHash <int, int>  hashNeuron;
			listNeuron.clear();
			hashNeuron.clear();

			//set node

			NeuronSWC S;
			S.n 	= 1;
			S.type 	= 3;
			S.x 	= seeds.at(0)->x;
			S.y 	= seeds.at(0)->y;
			S.z 	= seeds.at(0)->z;
			S.r 	= seeds.at(0)->r;
			S.pn 	= -1;
			listNeuron.append(S);
			hashNeuron.insert(S.n, listNeuron.size()-1);

			int* pi = new int[marknum];
			for(int i = 0; i< marknum;i++)
				pi[i] = 0;
			pi[0] = 1;
			int indexi,indexj;
			for(int loop = 0; loop<marknum;loop++)//���ѭ��ò���������·�����ȴӵ�1���㿪ʼ�������������һ���㣬Ȼ�������㿪ʼ��������㣬�Դ����ơ����Ӧ������С��������ʵ�ִ���
			{
				double min = 100000000;
				for(int i = 0; i<marknum; i++)
				{
					if (pi[i] == 1)
					{
						for(int j = 0;j<marknum; j++)
						{
							if(pi[j] == 0 && min > markEdge[i][j])
							{
								min = markEdge[i][j];
								indexi = i;
								indexj = j;//������ֵӦ�ñ�ʾ������̵Ķ�������
							}
						}
					}

				}
				if(indexi>=0)//�����if����о��γ���һ�������ڵ�ĸ��ڵ���������������Ľڵ㣬Ҳ������֮ǰ�����һ���ڵ�
				{
					S.n 	= indexj+1;
					S.type 	= 7;
					S.x 	= seeds.at(indexj)->x;
					S.y 	= seeds.at(indexj)->y;
					S.z 	= seeds.at(indexj)->z;
					S.r 	= seeds.at(indexj)->r;
					S.pn 	= indexi+1;
					listNeuron.append(S);
					hashNeuron.insert(S.n, listNeuron.size()-1);

				}else
				{
					break;
				}
				pi[indexj] = 1;
				indexi = -1;
				indexj = -1;
			}
			marker_MST.n = -1;
			marker_MST.on = true;
			marker_MST.listNeuron = listNeuron;
			marker_MST.hashNeuron = hashNeuron;

			QList<NeuronSWC> marker_MST_sorted;
			marker_MST_sorted.clear();
			if (SortSWC(marker_MST.listNeuron, marker_MST_sorted ,1, 0))//�������Ӧ���ǲ���Ӱ�츸�ڵ����ӽڵ�֮��Ĺ�ϵ��Ӧ��ֻ�ǽ���Щ�ڵ㰴��һ����˳��д��SWC�ļ���
			{
			}

			prepare_write(marker_MST_sorted);//��������ǽ�ÿһ����С�������ŵ�result_list���棬Ȼ�����ӡ

			if(markEdge) {delete []markEdge, markEdge = 0;}
			delete [] pi;
			seeds.clear();

		}
	}
}

QList<Node*> trim_nodes(QList<Node*> seed,V3DLONG sz_x,V3DLONG sz_y,V3DLONG sz_z)
{//��������������õ���ɾ��ֹ��ķ���һ��
	QList<Node*> result_seeds;
	QMultiMap<double,Node*> seed_radius;
	QMultiMap<V3DLONG,V3DLONG> parent_node;
	QMap<V3DLONG,int> flag;

	result_seeds.clear();
	seed_radius.clear();
	parent_node.clear();
	flag.clear();
	for(int i=0;i<seed.size();i++)
	{
		Node* elem1=seed.at(i);
		V3DLONG key=GET_IND(elem1->x,elem1->y,elem1->z);
		seed_radius.insert(elem1->r,elem1);
		flag.insert(GET_IND(elem1->x,elem1->y,elem1->z),0);
		for(int j=0;j<seed.size();j++)
		{
			Node* elem=seed.at(j);
			V3DLONG key1=GET_IND(elem->x,elem->y,elem->z);
			if((elem1->x==elem->x)&&(elem1->y==elem->y)&&(elem1->z==elem->z))
				continue;
			double dis=(double)sqrt((double)(elem->x-elem1->x)*(elem->x-elem1->x)+(double)(elem->y-elem1->y)*(elem->y-elem1->y)+(double)(elem->z-elem1->z)*(elem->z-elem1->z));
			if(dis/(elem->r+elem1->r)<1.2)
			{
				parent_node.insert(key,key1);//parent_root����Ӧ���ǰ��������еĸ��ڵ㣬��Map_rootnodeһ��

			}
		}

	}

	for(QMultiMap<double,Node*>::iterator iter=seed_radius.end();iter!=seed_radius.begin();iter--)
	{//�Ӱ뾶��Ŀ�ʼɾ�ڵ�
		if(iter==seed_radius.end())
			continue;
		Node* temp=iter.value();
		V3DLONG key2=GET_IND(temp->x,temp->y,temp->z);
		if(flag[GET_IND(temp->x,temp->y,temp->z)]==0)
		{
			result_seeds.append(temp);
			QMultiMap<V3DLONG,V3DLONG>::iterator begin=parent_node.lowerBound(key2);
			QMultiMap<V3DLONG,V3DLONG>::iterator end=parent_node.upperBound(key2);
			//printf("times:::%d\n",times);
			while((begin!=end))
			{
				flag[begin.value()]=1;//˵���ڱ��뾶��ĵ��Χ���Ͳ����ٳ�����ͼ����
				begin++;

			}
			flag[GET_IND(temp->x,temp->y,temp->z)]=1;

		}
	}

	return result_seeds;

}
void prepare_write(QList<NeuronSWC> marker_MST_sorted)
{
	//����дһ���ṹ�������е㶼�ӽ�ȥ
	V3DLONG n_temp=0;
	V3DLONG n_parent=0;
	QList<NeuronSWC> temp_marker_sorted;
	temp_marker_sorted.clear();

	if(result_list.size()==0)
	{
		n_temp=0;

	}else
	{
		n_temp=result_list.back().n;
		//printf("n_temp:::%d\n",n_temp);

	}

	for(int ii=0;ii<marker_MST_sorted.size();ii++)
	{
		NeuronSWC neuron=marker_MST_sorted.at(ii);
		neuron.n=n_temp+neuron.n;
		if(neuron.parent!=-1)
		{
			neuron.parent=n_temp+neuron.parent;

		}else
		{
			neuron.parent=n_temp+neuron.parent+2;

		}
		result_list.append(neuron);
		temp_marker_sorted.append(neuron);
	}
	

}


double cal_weight(V3DLONG curi,V3DLONG curj,V3DLONG curk, V3DLONG x,V3DLONG y,V3DLONG z,double inte_nd,double inte_cen,V3DLONG r)//MeanShift�㷨�м������ԭ��֮���Ȩֵ����ԭ��Խ����ȨֵԽ�����غ�ԭ��Խ����ȨֵԽ��
{
	//double r=100;
	double I=255;//�������ֵ
	double weight=0;
	double distance=(curi-x)/r*(curi-x)/r+(curj-y)/r*(curj-y)/r+(curk-z)/r*(curk-z)/r;
	double inten_sim=(inte_nd-inte_cen)/I*(inte_nd-inte_cen)/I;
	weight=exp(-1.0*sqrt(2*inten_sim+distance));
	//printf("%lf  %lf  %lf \n",distance,inten_sim,weight);
	return weight;
}

#define cal_core(cur,center,radius) exp(-0.1*(abs(cur-center)))

bool found_final(unsigned char * &img1d,V3DLONG x,V3DLONG y,V3DLONG z,V3DLONG sz_x,V3DLONG sz_y,V3DLONG sz_z,V3DLONG r)
{
	double intensity=img1d[GET_IND(x,y,z)];
	double sum1_z=0,sum2_z=0,sum1_y=0,sum2_y=0,sum1_x=0,sum2_x=0,w=0;
	//���巶Χ���԰뾶Ϊr��������������ƶ�
	V3DLONG xe=x-r;if(xe<0) xe=0;
	V3DLONG xb=x+r+1;if(xb>sz_x) xb=sz_x;
	V3DLONG ye=y-r;if(ye<0) ye=0;
	V3DLONG yb=y+r+1;if(yb>sz_y) yb=sz_y;
	V3DLONG ze=z-r;if(ze<0) ze=0;
	V3DLONG zb=z+r+1;if(zb>sz_z) zb=sz_z;
	for(V3DLONG i=xe;i<xb;i++) 
	{
		for(V3DLONG j=ye;j<yb;j++)
		{
			for(V3DLONG k=ze;k<zb;k++)
			{//Բ�ĵ���Ҫ����������е�ÿ���㶼����Ȩֵ

				if(GET_IND(i,j,k)==GET_IND(x,y,z)) continue;//Բ�ĵ㲻����
				if(img1d[GET_IND(i,j,k)]==0) continue;//����Ϊ0�ĵ㲻����
				double cur_intensity=img1d[GET_IND(i,j,k)];
				w=cal_weight(i,j,k,x,y,z,cur_intensity,intensity,r);//����������ÿ�����Բ�ĵ�Ȩ�أ���Ҫ�ǻ�������ǿ�Ⱥ;���

				double core_z=cal_core(k,z,r);//����˺�����z����
				sum1_z+=core_z*w*k;//����Z�����꣬�ۼӺ���ΪMeanShift��Ҫ���㹫ʽ�ķ���
				sum2_z+=core_z*w;//�ۼӺ���ΪMeanShift��Ҫ���㹫ʽ�ķ�ĸ
				//printf("%ld  %ld  %lf \n",k,cur_center->z,core_z);

				double core_y=cal_core(j,y,r);//y��
				sum1_y+=core_y*w*j;
				sum2_y+=core_y*w;

				double core_x=cal_core(i,x,r);//x��
				sum1_x+=core_x*w*i;
				sum2_x+=core_x*w;
				//printf("%ld   %ld  %ld  %lf  %lf  %lf  %lf  %lf  %lf  %lf  %lf  %lf\n",i,j,k,w,core_x,core_y,core_z,sum1_x,sum2_x,sum1_y,sum2_y,sum1_z,sum2_z);

			}
		}
	}

	V3DLONG next_x=(sum1_x/sum2_x)+0.5;
	V3DLONG next_y=(sum1_y/sum2_y)+0.5;
	V3DLONG next_z=(sum1_z/sum2_z)+0.5;
	if((next_x==x)&&(next_y==y)&&(next_z==z))
	{
		return false;//û�ж�

	}else
	{
		return true;//���ƶ�

	}


}

void meanshift_vn4(unsigned char * &img1d,V3DLONG x,V3DLONG y,V3DLONG z,V3DLONG sz_x,V3DLONG sz_y,V3DLONG sz_z,V3DLONG r,int iteration)
{//��vn3��ȣ�������vn3��ص��߼�
	int iter=0;
	int same_times=0;
	int situation=0;//���ڹ۲��ҵ������յ�������������

	QList<Node*> nodeList;
	nodeList.clear();//ÿ�ζ�clear
	Node *cur_center=new Node(x,y,z);//��ǰԲ�����ĵ㣬��¼Բ���ĵ����ά����
	double intensity=img1d[GET_IND(x,y,z)];//ͨ��Բ���ĵ���ά������������ź�ǿ��
	//cur_center->intensity=intensity;

	while(1)
	{
		if(iter==iteration)
		{
			break;

		}else
		{
			double sum1_z=0,sum2_z=0,sum1_y=0,sum2_y=0,sum1_x=0,sum2_x=0,w=0;
			//���巶Χ���԰뾶Ϊr��������������ƶ�
			V3DLONG xe=cur_center->x-r;if(xe<0) xe=0;
			V3DLONG xb=cur_center->x+r+1;if(xb>sz_x) xb=sz_x;
			V3DLONG ye=cur_center->y-r;if(ye<0) ye=0;
			V3DLONG yb=cur_center->y+r+1;if(yb>sz_y) yb=sz_y;
			V3DLONG ze=cur_center->z-r;if(ze<0) ze=0;
			V3DLONG zb=cur_center->z+r+1;if(zb>sz_z) zb=sz_z;
			for(V3DLONG i=xe;i<xb;i++) 
			{
				for(V3DLONG j=ye;j<yb;j++)
				{
					for(V3DLONG k=ze;k<zb;k++)
					{//Բ�ĵ���Ҫ����������е�ÿ���㶼����Ȩֵ

						if(GET_IND(i,j,k)==GET_IND(cur_center->x,cur_center->y,cur_center->z)) continue;//Բ�ĵ㲻����
						if(img1d[GET_IND(i,j,k)]==0) continue;//����Ϊ0�ĵ㲻����
						double cur_intensity=img1d[GET_IND(i,j,k)];
						w=cal_weight(i,j,k,cur_center->x,cur_center->y,cur_center->z,cur_intensity,intensity,r);//����������ÿ�����Բ�ĵ�Ȩ�أ���Ҫ�ǻ�������ǿ�Ⱥ;���

						double core_z=cal_core(k,cur_center->z,r);//����˺�����z����
						sum1_z+=core_z*w*k;//����Z�����꣬�ۼӺ���ΪMeanShift��Ҫ���㹫ʽ�ķ���
						sum2_z+=core_z*w;//�ۼӺ���ΪMeanShift��Ҫ���㹫ʽ�ķ�ĸ
						//printf("%ld  %ld  %lf \n",k,cur_center->z,core_z);

						double core_y=cal_core(j,cur_center->y,r);//y��
						sum1_y+=core_y*w*j;
						sum2_y+=core_y*w;

						double core_x=cal_core(i,cur_center->x,r);//x��
						sum1_x+=core_x*w*i;
						sum2_x+=core_x*w;
						//printf("%ld   %ld  %ld  %lf  %lf  %lf  %lf  %lf  %lf  %lf  %lf  %lf\n",i,j,k,w,core_x,core_y,core_z,sum1_x,sum2_x,sum1_y,sum2_y,sum1_z,sum2_z);

					}
				}
			}
			V3DLONG temp_x=cur_center->x;//3��temp����ǰһ��Բ�ĵ�x\y\z����
			V3DLONG temp_y=cur_center->y;
			V3DLONG temp_z=cur_center->z;
			flag[GET_IND(temp_x,temp_y,temp_z)]=1;//����һ��Բ�ĵ���Ϊ1����ʾ�õ��Ѿ���������

			V3DLONG next_x=(sum1_x/sum2_x)+0.5;
			V3DLONG next_y=(sum1_y/sum2_y)+0.5;
			V3DLONG next_z=(sum1_z/sum2_z)+0.5;
			cur_center->x=next_x;//���µ���һ��Բ�ĵ㸳����Ϊ��ǰԲ��
			cur_center->y=next_y;
			cur_center->z=next_z;
			intensity=img1d[GET_IND(cur_center->x,cur_center->y,cur_center->z)];//���µ�ǰԲ�ĵ�����ǿ��


			//��ʽ2
			Node *pre_center=new Node(temp_x,temp_y,temp_z);
			if(!nodeList.contains(pre_center))//�ж���һ��Բ���Ƿ���List���棬����ڵĻ�˵����ǰ�ڵ��ǰһ�ڵ���ͬ��ǰһ�ڵ㲢δ�ƶ�
			{	//�����İ뾶
				enlarge_radiusof_single_node_xy(img1d,pre_center,sz_x,sz_y,sz_z);//����ÿһ����İ뾶�����ܻ�Ը��ڵ�����ظ�����
				//printf("%lf\n",pre_center->r);
				nodeList.append(pre_center);
			}	

			if(flag[GET_IND(cur_center->x,cur_center->y,cur_center->z)]==1)
			{//��������ĵ�ǰ���ĵ��Ѿ��������������whileѭ�������ٽ��м���
				//�������˵����ǰ�ͱ������һ�Σ��ټ���Ļ����ǻᱻ�ƶ����յ㣬����������
				break;
			}

			iter++;//���մ�ʩ������100�κ󣬲��ٵ���

		}
	}

	bool f=found_final(img1d,cur_center->x,cur_center->y,cur_center->z,sz_x,sz_y,sz_z,r);//�����жϸõ��Ƿ����ƶ���Ҳ��ʾ������Ǹ��ڵ㻹��·���ϵĵ�
	V3DLONG ind2=GET_IND(cur_center->x,cur_center->y,cur_center->z);

	double temp_dis=1000000;

	{//�ҵ����µĸ����ܶ����ĵ�
		//situation=1;//���ڱ���ҵ������յ������������
		int mark=0;
		if(Map_finalnode.contains(ind2))//20150311,�����Map_finalnode���棬˵�����ܱ��ƶ��ˣ����յ�
		{//���Map_finalnode�������������ڵ㣬�����List�ŵ���Ӧ��Map_finalnode_List��
			for(int iii=0;iii<nodeList.size();iii++)
			{
				V3DLONG node_x=nodeList.at(iii)->x;
				V3DLONG node_y=nodeList.at(iii)->y;
				V3DLONG node_z=nodeList.at(iii)->z;
				Map_nodes[GET_IND(node_x,node_y,node_z)]=Map_finalnode[ind2];//��nodeList����Ľڵ㸳����Ӧ���ڵ�ı��
				Map_allnodes.insert(Map_finalnode[ind2],nodeList.at(iii));
				Map_finalnode_list[Map_finalnode[ind2]].append(nodeList.at(iii));

			}

		}else if(Map_nodes.contains(GET_IND(cur_center->x,cur_center->y,cur_center->z)))//�յ����ƶ������ұ�������
		{
			//�����������꣬�ҵ�������mark,��nodeList��ֵ
			situation=2;
			int mark3=Map_nodes[GET_IND(cur_center->x,cur_center->y,cur_center->z)];

			for(int iii=0;iii<nodeList.size();iii++)
			{
				V3DLONG node_x2=nodeList.at(iii)->x;
				V3DLONG node_y2=nodeList.at(iii)->y;
				V3DLONG node_z2=nodeList.at(iii)->z;
				Map_nodes[GET_IND(node_x2,node_y2,node_z2)]=mark3;//��nodeList����Ľڵ㸳����Ӧ���ڵ�ı��
				Map_allnodes.insert(mark3,nodeList.at(iii));
				Map_finalnode_list[mark3].append(nodeList.at(iii));

			}

		}
		else
		{
			int new_mark=Map_rootnode.size()+1;
			cur_center->class_mark=new_mark;

			enlarge_radiusof_single_node_xy(img1d,cur_center,sz_x,sz_y,sz_z);//������ڵ�İ뾶
			
			Map_rootnode.insert(new_mark,cur_center);

			for(int iii=0;iii<nodeList.size();iii++)
			{
				V3DLONG node_x1=nodeList.at(iii)->x;
				V3DLONG node_y1=nodeList.at(iii)->y;
				V3DLONG node_z1=nodeList.at(iii)->z;

				Map_nodes[GET_IND(node_x1,node_y1,node_z1)]=new_mark;//��nodeList����Ľڵ㸳����Ӧ���ڵ�ı��
				Map_allnodes.insert(new_mark,nodeList.at(iii));
				Map_finalnode_list[new_mark].append(nodeList.at(iii));

			}
		}

	}

}

int meanshift_plugin(const V3DPluginArgList & input, V3DPluginArgList & output)
{
	cout<<"Welcome to image_threshold function"<<endl;
	if(input.size() != 2 || output.size() != 1) 
	{
		cout<<"illegal input!"<<endl;
		printHelp();
		return -1;
	}


	// 1 - Read input image
	vector<char*>* inlist = (vector<char*>*)(input.at(0).p);
	if (inlist->size() != 1)
	{
		cout<<"You must specify 1 input file!"<<endl;
		return -1;
	}
	char * infile = inlist->at(0);
	cout<<"input file: "<<infile<<endl;
	unsigned char * inimg1d = NULL;
	V3DLONG * sz = NULL;
	int datatype;
	if (!loadImage(infile, inimg1d, sz, datatype)) return -1;


	// 2 - Read color channel and threshold parameter
	vector<char*>* paralist = (vector<char*>*)(input.at(1).p);
	if (paralist->size() != 2)
	{
		cout<<"Illegal parameter!"<<endl;
		printHelp();
		return -1;
	}
	int c = atoi(paralist->at(0));
	int thres = atoi(paralist->at(1));
	cout<<"color channel: "<<c<<endl;
	cout<<"threshold : "<<thres<<endl;
	if (c < 0 || c>=sz[3])
	{
		cout<<"The color channel does not exist!"<<endl;
		return -1;
	}
	loadImage(infile, inimg1d, sz, datatype, c);


	// 3 - Read output fileName
	vector<char*>* outlist = (vector<char*>*)(output.at(0).p);
	if (outlist->size() != 1)
	{
		cout<<"You must specify 1 output file!"<<endl;
		return -1;
	}
	char * outfile = outlist->at(0);
	cout<<"output file: "<<outfile<<endl;

	// 4 - Do binary segmentation
	V3DLONG tb = sz[0]*sz[1]*sz[2]*datatype;
	unsigned char * nm = NULL;
	try {
		nm = new unsigned char [tb];
	} catch (...) {
		throw("Fail to allocate memory in Image Thresholding plugin.");
	}
	for (V3DLONG i=0;i<tb;i++)
	{
		if (inimg1d[i]>=thres) nm[i] = 255;
		else nm[i] = 0;
	}

	// 5 - Save file and free memory
	sz[3] = 1;
	saveImage(outfile, nm, sz, datatype);
	if (nm) {delete []nm; nm=NULL;}

	return 1;
}

void printHelp()
{
	cout<<"\nThis is a demo plugin to perform binary thresholding in an image. by Yinan Wan 2012-02"<<endl;
	cout<<"\nUsage: v3d -x <example_plugin_name> -f image_thresholding -i <input_image_file> -o <output_image_file> -p <subject_color_channel> <threshold>"<<endl;
	cout<<"\t -i <input_image_file>                      input 3D image (tif, raw or lsm)"<<endl;
	cout<<"\t -o <output_image_file>                     output image of the thresholded subject channel"<<endl;
	cout<<"\t -p <subject_color_channel> <threshold>     the channel you want to perform thresholding and the threshold"<<endl;
	cout<<"\t                                            the 2 paras must come in this order"<<endl;
	cout<<"\nDemo: v3d -x libexample_debug.dylib -f image_thresholding -i input.tif -o output.tif -p 0 100\n"<<endl;
	return;
}

