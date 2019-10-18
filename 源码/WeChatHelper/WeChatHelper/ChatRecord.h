#pragma once
#include <string>

void HookChatRecord();		//HOOK聊天记录
void RecieveWxMesage();		//接收微信消息
void SendWxMessage();		//发送微信消息到客户端
bool FilterGroupID(wchar_t *GroupID);		//指定群ID的消息发送出去
bool FilterGroupMessage(wchar_t *Messagecontent); //过滤群消息内容
std::wstring GetMsgByAddress(DWORD memAddress);	//从地址中获取消息内容
