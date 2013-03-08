#include "defs.h"
struct multicast_error_data
{//IGMP������Ϣ����������
	u_long temp1;
	u_long temp2;
	u_char message;	/*Ϊ1��ʾû��MFC��2��ʾMFC����*/
	u_char temp3;
	u_char vif;/*�յ������鲥���ĵĽӿں�*/
	u_char temp4;
	struct in_addr source;/*�յ���Դ�����鲥��Դ*/
	struct in_addr destination;/*�鲥��ַ*/
};
int check_incoming(vifi_t vif,u_int32 address)
{//ȷ��ĳ���ӿ��Ƿ񿿽��鲥Դ������ǵĻ�����1�����ǵĻ�����0
	u_int32 source;
	vifi_t rightvif;

	source=address;
	rightvif=get_incoming_vif(source);
	if(rightvif==vif)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}
void multicast_packet_error()
{//���յ�һ���鲥���ĵ�ʱ������ں�û����Ӧ��MFC������MFC�����򷵻�һ�������IGMP��Ϣ����ΪIGMP������Ϣ
	struct multicast_error_data *mydata;
	u_int32 source,group;
	vifi_t vif;
	struct mrt *mymrt;
	mydata=(struct multicast_error_data *)igmp_receive_buf;
	if(mydata->message==1)
	{//MFC��û������鲥���ĵ�ת���������
		if(check_incoming(mydata->vif,mydata->source.s_addr)==1)
		{//������Ǵӿ����鲥Դ�Ľӿ��յ��Ļ�
			source=mydata->source.s_addr;
			group=mydata->destination.s_addr;
		
			log(LOG_INFO,"We Receive a new multicast packet from %s group %s,create the (S,G) entry and forward this packet!",inet_fmt(source,s1),inet_fmt(group,s1));
			add_sg(source,group,mydata->vif);/*��������鲥����Ŀ*/
//log(LOG_INFO,"Receive multicast packet from %s",myvifs[mydata->vif].name);
			change_mfc(igmpsocket,source,group,mydata->vif);
		}
		else
		{
			log(LOG_INFO,"This new multicast packet is from wrong interface!Ignore it");
		}
	}
	if(mydata->message==2)
	{
		source=mydata->source.s_addr;
		group=mydata->destination.s_addr;
		vif=mydata->vif;

		mymrt=find_sg(source,group);
		if(mymrt==NULL)
		{
			return;
		}
		log(LOG_INFO,"Receive multicast packet from %s with source %s",myvifs[vif].name,inet_fmt(source,s1));
		print_sg();
		if(mymrt->outnumber!=0)
		{
			log(LOG_INFO,"Send Assert packet!");
			pim_assert_output(source,group,vif,mymrt->preference,mymrt->metric);
		}
	}
}