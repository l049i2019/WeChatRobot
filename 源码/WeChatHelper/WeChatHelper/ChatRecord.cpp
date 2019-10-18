#include "stdafx.h"
#include <string>
#include <Shlwapi.h>
#include "ChatRecord.h"
#include "FriendList.h"
#include "CAutoFunction.h"
#include <stdio.h>
#pragma comment(lib,"Shlwapi.lib")
using namespace std;


BOOL g_AutoChat = FALSE;	//�Ƿ��Զ�����
BOOL isSendTuLing = FALSE;	//�Ƿ��Ѿ�������ͼ�������
BOOL isText = TRUE;			//�Ƿ���������Ϣ
wchar_t tempwxid[50] = { 0 };	//���΢��ID

DWORD r_esp = 0;
DWORD r_eax = 0;

CHAR originalCode[5] = { 0 };
//������ƫ��
DWORD dwParam = (DWORD)GetModuleHandle(L"WeChatWin.dll") + ReciveMessageParam;

//������ҪHOOK�ĵ�ַ
DWORD dwHookAddr = (DWORD)GetModuleHandle(L"WeChatWin.dll") + ReciveMessage - 5;

//���ص�ַ
DWORD RetAddr = dwHookAddr + 5;

//��Ϣ����
const unsigned int uiMsgLength = 2000;

//��Ϣ�ṹ��
struct Message
{
	wchar_t type[10];		//��Ϣ����
	wchar_t source[20];		//��Ϣ��Դ
	wchar_t wxid[40];		//΢��ID/ȺID
	wchar_t msgSender[40];	//��Ϣ������
	wchar_t content[uiMsgLength];	//��Ϣ����
	BOOL isMoney = FALSE;	//�Ƿ����տ���Ϣ
};


//************************************************************
// ��������: HookChatRecord
// ����˵��: HOOK�����¼
// ��    ��: GuiShou
// ʱ    ��: 2019/7/6
// ��    ��: void
// �� �� ֵ: void 
//************************************************************
void HookChatRecord()
{
	//��װ����
	BYTE bJmpCode[5] = { 0xE9 };
	*(DWORD*)&bJmpCode[1] = (DWORD)RecieveWxMesage - dwHookAddr - 5;

	//���浱ǰλ�õ�ָ��,��unhook��ʱ��ʹ�á�
	ReadProcessMemory(GetCurrentProcess(), (LPVOID)dwHookAddr, originalCode, 5, 0);

	//����ָ�� B9 E8CF895C //mov ecx,0x5C89CFE8
	WriteProcessMemory(GetCurrentProcess(), (LPVOID)dwHookAddr, bJmpCode, 5, 0);
}



//************************************************************
// ��������: RecieveMesage
// ����˵��: ������Ϣ
// ��    ��: GuiShou
// ʱ    ��: 2019/7/6
// ��    ��: void
// �� �� ֵ: void 
//************************************************************
__declspec(naked) void RecieveWxMesage()
{
	//�����ֳ�
	__asm
	{
		//���䱻���ǵĴ���
		//5B950573  |.  B9 E8CF895C           mov ecx,WeChatWi.5C89CFE8
		//mov ecx,0x5C89CFE8
		mov ecx, dwParam

		//��ȡesp�Ĵ������ݣ�����һ��������
		mov r_esp, esp
		mov r_eax, eax

		pushad
		pushfd
	}
	SendWxMessage();

	//�ָ��ֳ�
	__asm
	{
		popfd
		popad
		//���ر�HOOKָ�����һ��ָ��
		jmp RetAddr
	}
}


//************************************************************
// ��������: SendMessage
// ����˵��: �����յ�����Ϣ���͸��ͻ���
// ��    ��: GuiShou
// ʱ    ��: 2019/7/6
// ��    ��: void
// �� �� ֵ: void 
//************************************************************
void SendWxMessage()
{
	Message *msg=new Message;
	//��Ϣ���λ��
	DWORD** msgAddress = (DWORD * *)r_esp;
	//��Ϣ����
	DWORD msgType = *((DWORD*)(**msgAddress + 0x30));
	
	BOOL isFriendMsg = FALSE;		//�Ƿ��Ǻ�����Ϣ
	BOOL isImageMessage = FALSE;	//�Ƿ���ͼƬ��Ϣ
	BOOL isRadioMessage = FALSE;	//�Ƿ�����Ƶ��Ϣ
	BOOL isVoiceMessage = FALSE;	//�Ƿ���������Ϣ
	BOOL isBusinessCardMessage = FALSE;	//�Ƿ�����Ƭ��Ϣ
	BOOL isExpressionMessage = FALSE;	//�Ƿ�����Ƭ��Ϣ
	BOOL isLocationMessage = FALSE;	//�Ƿ���λ����Ϣ
	BOOL isSystemMessage = FALSE;	//�Ƿ���ϵͳ������Ϣ
	BOOL isFriendRequestMessage = FALSE;	//�Ƿ��Ǻ���������Ϣ
	BOOL isOther = FALSE;	//�Ƿ���������Ϣ
	BOOL isTextMessage = FALSE;//�Ƿ���������Ϣ


	switch (msgType)
	{
	case 0x01:
		memcpy(msg->type, L"����",sizeof(L"����"));
		isTextMessage = TRUE;
		break;
	case 0x03:
		memcpy(msg->type, L"ͼƬ", sizeof(L"ͼƬ"));
		isImageMessage = TRUE;
		break;
	case 0x22:
		memcpy(msg->type, L"����", sizeof(L"����"));
		isVoiceMessage = TRUE;
		break;
	case 0x25:
		memcpy(msg->type, L"����ȷ��", sizeof(L"����ȷ��"));
		isFriendRequestMessage = TRUE;
		break;
	case 0x28:
		memcpy(msg->type, L"POSSIBLEFRIEND_MSG", sizeof(L"POSSIBLEFRIEND_MSG"));
		isOther = TRUE;
		break;
	case 0x2A:
		memcpy(msg->type, L"��Ƭ", sizeof(L"��Ƭ"));
		isBusinessCardMessage = TRUE;
		break;
	case 0x2B:
		memcpy(msg->type, L"��Ƶ", sizeof(L"��Ƶ"));
		isRadioMessage = TRUE;
		break;
	case 0x2F:
		//ʯͷ������
		memcpy(msg->type, L"����", sizeof(L"����"));
		isExpressionMessage = TRUE;
		break;
	case 0x30:
		memcpy(msg->type, L"λ��", sizeof(L"λ��"));
		isLocationMessage = TRUE;
		break;
	case 0x31:
		//����ʵʱλ��
		//�ļ�
		//ת��
		//����
		//�տ�
		memcpy(msg->type, L"����ʵʱλ�á��ļ���ת�ˡ�����", sizeof(L"����ʵʱλ�á��ļ���ת�ˡ�����"));
		isOther = TRUE;
		break;
	case 0x32:
		memcpy(msg->type, L"VOIPMSG", sizeof(L"VOIPMSG"));
		isOther = TRUE;
		break;
	case 0x33:
		memcpy(msg->type, L"΢�ų�ʼ��", sizeof(L"΢�ų�ʼ��"));
		isOther = TRUE;
		break;
	case 0x34:
		memcpy(msg->type, L"VOIPNOTIFY", sizeof(L"VOIPNOTIFY"));
		isOther = TRUE;
		break;
	case 0x35:
		memcpy(msg->type, L"VOIPINVITE", sizeof(L"VOIPINVITE"));
		isOther = TRUE;
		break;
	case 0x3E:
		memcpy(msg->type, L"С��Ƶ", sizeof(L"С��Ƶ"));
		isRadioMessage = TRUE;
		break;
	case 0x270F:
		memcpy(msg->type, L"SYSNOTICE", sizeof(L"SYSNOTICE"));
		isOther = TRUE;
		break;
	case 0x2710:
		//ϵͳ��Ϣ
		//���
		memcpy(msg->type, L"�����ϵͳ��Ϣ", sizeof(L"�����ϵͳ��Ϣ"));
		isSystemMessage = TRUE;
		break;
	default:
		isOther = TRUE;
		break;
	}
	//��Ϣ����
	wstring fullmessgaedata = GetMsgByAddress(**msgAddress + 0x68);	//��������Ϣ����
	//�ж���Ϣ��Դ��Ⱥ��Ϣ���Ǻ�����Ϣ
	wstring msgSource2 = L"<msgsource />\n";
	wstring msgSource = L"";
	msgSource.append(GetMsgByAddress(**msgAddress + 0x168));
	//������Ϣ
	if (msgSource.length() <= msgSource2.length())
	{
		memcpy(msg->source, L"������Ϣ", sizeof(L"������Ϣ"));
		isFriendMsg = TRUE;
	}
	else
	{
		//Ⱥ��Ϣ
		memcpy(msg->source, L"Ⱥ��Ϣ", sizeof(L"Ⱥ��Ϣ"));
	}

	//��ʾ΢��ID/ȺID
	LPVOID pWxid = *((LPVOID *)(**msgAddress + 0x40));
	swprintf_s(msg->wxid, L"%s", (wchar_t*)pWxid);

	//�����Ⱥ��Ϣ
	if (isFriendMsg == FALSE)
	{
		//��ʾ��Ϣ������
		LPVOID pSender = *((LPVOID *)(**msgAddress + 0x114));
		swprintf_s(msg->msgSender, L"%s", (wchar_t*)pSender);
	}
	else
	{
		memcpy(msg->msgSender, L"NULL", sizeof(L"NULL"));
	}


	//��ʾ��Ϣ����  �����޷���ʾ����Ϣ ��ֹ����
	if (StrStrW(msg->wxid, L"gh"))
	{
		//�����ͼ������˷�������Ϣ ������Ϣ�Ѿ����͸�ͼ�������
		if ((StrCmpW(msg->wxid,L"gh_ab370b2e4b62")==0)&&isSendTuLing==TRUE)
		{
			wchar_t tempcontent[0x200] = { 0 };
			//�����жϻ����˻ظ�����Ϣ���� ����������� ֱ�ӻظ�
			if (msgType!=0x01)
			{
				SendTextMessage(tempwxid, (wchar_t*)L"������");
				isSendTuLing = FALSE;
			}
			//�ٴ��жϷ��͸������˵���Ϣ����
			else if (isText == FALSE)
			{
				SendTextMessage(tempwxid, (wchar_t*)L"�� ��֧�ִ�����ϢŶ �뷢���� ôô��");
				isSendTuLing = FALSE;
				isText = TRUE;
			}
			else   //��������� �ٴ��жϳ���
			{
				//�����õ���Ϣ���� ���͸�����
				LPVOID pContent = *((LPVOID *)(**msgAddress + 0x68));
				swprintf_s(tempcontent, L"%s", (wchar_t*)pContent);
				//�жϷ��ص���Ϣ�Ƿ����
				if (wcslen(tempcontent) > 0x100)
				{
					wcscpy_s(tempcontent, wcslen(L"������"), L"������");
				}

				SendTextMessage(tempwxid, tempcontent);
				isSendTuLing = FALSE;
			}
		}
		//���΢��IDΪgh_3dfda90e39d6 ˵�����տ���Ϣ
		else if ((StrCmpW(msg->wxid, L"gh_3dfda90e39d6") == 0))
		{
			swprintf_s(msg->content, L"%s", L"΢���տ��");
			msg->isMoney = TRUE;
		}
		else
		{
			//���΢��ID�д���gh ˵���ǹ��ں�
			swprintf_s(msg->content, L"%s", L"���ںŷ�������,�����ֻ��ϲ鿴");
		}
	}
	//����ͼƬ��Ϣ 
	else if (isImageMessage == TRUE)
	{
		swprintf_s(msg->content, L"%s", L"�յ�ͼƬ��Ϣ,�����ֻ��ϲ鿴");
	}
	else if (isRadioMessage ==TRUE)
	{
		swprintf_s(msg->content, L"%s", L"�յ���Ƶ��Ϣ,�����ֻ��ϲ鿴");
	}
	else if (isVoiceMessage==TRUE)
	{
		swprintf_s(msg->content, L"%s", L"�յ�������Ϣ,�����ֻ��ϲ鿴");
	}
	else if (isBusinessCardMessage == TRUE)
	{
		swprintf_s(msg->content, L"%s", L"�յ���Ƭ��Ϣ,���Զ���Ӻ���");
		//�Զ���Ӻ���
		AutoAddCardUser(fullmessgaedata);
	}
	else if (isExpressionMessage == TRUE)
	{
		swprintf_s(msg->content, L"%s", L"�յ�������Ϣ,�����ֻ��ϲ鿴");
	}
	//�Զ�ͨ����������
	else if(isFriendRequestMessage==TRUE)
	{
		swprintf_s(msg->content, L"%s", L"�յ���������,���Զ�ͨ��");
		//�Զ�ͨ����������
		AutoAgreeUserRequest(fullmessgaedata);
	}
	else if (isOther == TRUE)
	{
		//ȡ����Ϣ����
		wchar_t tempcontent[0x10000] = { 0 };
		LPVOID pContent = *((LPVOID *)(**msgAddress + 0x68));
		swprintf_s(tempcontent, L"%s", (wchar_t*)pContent);
		//�ж��Ƿ���ת����Ϣ
		if (StrStrW(tempcontent,L"΢��ת��"))
		{
			swprintf_s(msg->content, L"%s", L"�յ�ת����Ϣ,���Զ��տ�");

			//�Զ��տ�
			AutoCllectMoney(fullmessgaedata, msg->wxid);	
		}
		else
		{
			//�ж���Ϣ���� ������ȳ����Ͳ���ʾ
			if (wcslen(tempcontent) > 200)
			{
				swprintf_s(msg->content, L"%s", L"��Ϣ���ݹ��� �Ѿ�����");
			}
			else
			{
				//�ж��Ƿ���ת����Ϣ
				swprintf_s(msg->content, L"%s", L"�յ�����ʵʱλ�á��ļ������ӵ�������Ϣ,�����ֻ��ϲ鿴");
			}
			
		}
	}
	else if (isLocationMessage == TRUE)
	{
		swprintf_s(msg->content, L"%s", L"�յ�λ����Ϣ,�����ֻ��ϲ鿴");
	}
	else if (isSystemMessage == TRUE)
	{
		wchar_t tempbuff[0x1000];
		LPVOID pContent = *((LPVOID *)(**msgAddress + 0x68));
		swprintf_s(tempbuff, L"%s", (wchar_t*)pContent);

		//�����ﴦ�����Ⱥ����Ϣ
		if ((StrStrW(tempbuff, L"�Ƴ���Ⱥ��")||StrStrW(tempbuff, L"������Ⱥ��")))
		{
			wcscpy_s(msg->content, wcslen(tempbuff) + 1, tempbuff);
		}
		else
		{
			swprintf_s(msg->content, L"%s", L"�յ������ϵͳ��Ϣ,�����ֻ��ϲ鿴");
		}
	
	}
	//������������Ϣ֮��
	else
	{
		wchar_t tempbuff[0x1000];
		LPVOID pContent = *((LPVOID *)(**msgAddress + 0x68));
		swprintf_s(tempbuff, L"%s", (wchar_t*)pContent);
		//�ж���Ϣ���� ������ȳ����Ͳ���ʾ
		if (wcslen(tempbuff)>uiMsgLength)
		{
			swprintf_s(msg->content, L"%s", L"��Ϣ���ݹ��� �Ѿ�����");
		}
		else
		{
			swprintf_s(msg->content, L"%s", (wchar_t*)pContent);
		}

	}
	if ((TRUE == isFriendMsg) || (FALSE == isTextMessage))
	{
		delete msg;
		msg = NULL;
		return;
	}
	else if(!FilterGroupID(msg->wxid))
	{
		delete msg;
		msg = NULL;
		return;
	}
	else if (!FilterGroupMessage(msg->content))
	{

		delete msg;
		msg = NULL;
		return;
	}
	

	//���͵����ƶ�
	HWND hWnd = FindWindow(NULL, TEXT("΢������"));
	if (hWnd == NULL)
	{
		OutputDebugStringA("δ���ҵ�΢�����ִ���");
	}

	/*COPYDATASTRUCT chatmsg;
	chatmsg.dwData = WM_ShowChatRecord;//����һ����ֵ, ������������־��
	chatmsg.cbData = sizeof(Message);// strlen(szSendBuf);//�����͵����ݵĳ�
	chatmsg.lpData = msg;// szSendBuf;//�����͵����ݵ���ʼ��ַ(����ΪNULL)
	SendMessage(hWnd, WM_COPYDATA, (WPARAM)hWnd, (LPARAM)&chatmsg);*/
	SendTextMessage((wchar_t*)L"badminton", msg->content);

	//���ﴦ���Զ�����
	/*if (isFriendMsg == TRUE && g_AutoChat == TRUE)
	{
		//�ж���Ϣ����
		if (msgType != 0x01)
		{
			isText = FALSE;
		}
		//����һ��΢��ID
		wcscpy_s(tempwxid, wcslen(msg->wxid) + 1, msg->wxid);

		//�ж���Ϣ�����Ƿ������
		if (wcscmp(msg->content, L"���") == 0)
		{
			swprintf_s(msg->content, L"%s", L"���");
		}

		

		//�õ���Ϣ���� ����ͼ�������
		SendTextMessage((wchar_t*)L"gh_ab370b2e4b62", msg->content);
		isSendTuLing = TRUE;
	}*/
	delete msg;
	msg = NULL;
}
//************************************************************
// ��������: FilterGroupID
// ����˵��: ֻ����ָ��Ⱥ����Ϣ
// ��    ��: lijie
// ʱ    ��: 2019/10/16
// ��    ��: wchar_t *GroupID ȺID
// �� �� ֵ: bool �Ƿ�����Ϣ
//************************************************************
bool FilterGroupID(wchar_t *GroupID)
{
	
	/*if ((StrCmpW(GroupID, L"6569692106@chatroom") == 0)    //������԰��ë��Ⱥ
		|| (StrCmpW(GroupID, L"4796739218@chatroom") == 0) //����7Cס��Ⱥ
		|| (StrCmpW(GroupID, L"5076739206@chatroom") == 0))//������԰ҵ��Ⱥ
	{
		return true;
	}
	else
		return false;*/
	return true;
}
//************************************************************
// ��������: FilterGroupMessage
// ����˵��: ����Ⱥ����Ϣ
// ��    ��: lijie
// ʱ    ��: 2019/10/16
// ��    ��: wchar_t *Messagecontent Ⱥ��Ϣ����
// �� �� ֵ: bool �Ƿ�����Ϣ
//************************************************************
bool FilterGroupMessage(wchar_t *Messagecontent)
{
	if (StrStrW(Messagecontent, L"ס����")
		|| StrStrW(Messagecontent, L"�鷿")
		)
	{
		return true;
	}
	else
		return false;
}

//************************************************************
// ��������: GetMsgByAddress
// ����˵��: �ӵ�ַ�л�ȡ��Ϣ����
// ��    ��: GuiShou
// ʱ    ��: 2019/7/6
// ��    ��: DWORD memAddress  Ŀ���ַ
// �� �� ֵ: LPCWSTR	��Ϣ����
//************************************************************
std::wstring GetMsgByAddress(DWORD memAddress)
{
	wstring tmp;
	DWORD msgLength = *(DWORD*)(memAddress + 4);
	if (msgLength > 0) {
		WCHAR* msg = new WCHAR[msgLength + 1]{ 0 };
		wmemcpy_s(msg, msgLength + 1, (WCHAR*)(*(DWORD*)memAddress), msgLength + 1);
		tmp = msg;
		delete[]msg;
	}
	return  tmp;
}


