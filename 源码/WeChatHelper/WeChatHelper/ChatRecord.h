#pragma once
#include <string>

void HookChatRecord();		//HOOK�����¼
void RecieveWxMesage();		//����΢����Ϣ
void SendWxMessage();		//����΢����Ϣ���ͻ���
bool FilterGroupID(wchar_t *GroupID);		//ָ��ȺID����Ϣ���ͳ�ȥ
bool FilterGroupMessage(wchar_t *Messagecontent); //����Ⱥ��Ϣ����
std::wstring GetMsgByAddress(DWORD memAddress);	//�ӵ�ַ�л�ȡ��Ϣ����
