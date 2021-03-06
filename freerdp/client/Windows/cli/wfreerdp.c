/**
 * FreeRDP: A Remote Desktop Protocol Implementation
 * Windows Client
 *
 * Copyright 2009-2011 Jay Sorg
 * Copyright 2010-2011 Vic Lee
 * Copyright 2010-2011 Marc-Andre Moreau <marcandre.moreau@gmail.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <winpr/windows.h>

#include <winpr/crt.h>
#include <winpr/credui.h>

// Apex {{
#include <Shlwapi.h>
#pragma comment(lib, "shlwapi.lib")
// }}

#include <freerdp/freerdp.h>
#include <freerdp/constants.h>

#include <freerdp/client/file.h>
#include <freerdp/client/cmdline.h>
#include <freerdp/client/channels.h>
#include <freerdp/channels/channels.h>

#include "resource.h"

#include "wf_client.h"

#include <shellapi.h>

 /*
 rdp-player command line:
 1. rdp-player.exe rdp-record-filename.tpr
 2. rdp-player.exe http://xxx.xxx.xx.xx/xx/tp-rdp.tpr
 */

INT WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	int status = 0;
	HANDLE thread = NULL;
	wfContext* wfc = NULL;
	DWORD dwExitCode = 0;
	rdpContext* context = NULL;
	rdpSettings* settings = NULL;
	RDP_CLIENT_ENTRY_POINTS clientEntryPoints;
	int ret = 1;
	int argc = 0, i;
	LPWSTR* args = NULL;
	LPWSTR cmd = NULL;
	char** argv = NULL;

	ZeroMemory(&clientEntryPoints, sizeof(RDP_CLIENT_ENTRY_POINTS));
	clientEntryPoints.Size = sizeof(RDP_CLIENT_ENTRY_POINTS);
	clientEntryPoints.Version = RDP_CLIENT_INTERFACE_VERSION;

// Apex {{
	// 	RdpClientEntry(&clientEntryPoints);
	// 
	// 	context = freerdp_client_context_new(&clientEntryPoints);
	// 	if (!context)
	// 		return -1;
// }}

	cmd = GetCommandLineW();

	if (!cmd)
		goto out;

	args = CommandLineToArgvW(cmd, &argc);
	if (!args)
	// Apex {{
	{
		MessageBox(NULL, _T("命令行错误！"), _T("错误"), MB_OK | MB_ICONERROR);
		goto out;
	}
	// Apex }}

	argv = calloc(argc, sizeof(char*));
	if (!argv)
		goto out;

	// Apex {{
	if (argc < 2)
	{
		MessageBox(NULL, _T("请指定要播放的 Teleport RDP 录像文件！"), _T("提示"), MB_OK | MB_ICONWARNING);
		goto out;
	}
	// }}

	for (i = 0; i < argc; i++)
	{
	// Apex {{
		int size = WideCharToMultiByte(CP_ACP, 0, args[i], -1, NULL, 0, NULL, NULL);
	// }}
		argv[i] = calloc(size, sizeof(char));
		if (!argv[i])
			goto out;

	// Apex {{
		if (WideCharToMultiByte(CP_ACP, 0, args[i], -1, argv[i], size, NULL, NULL) != size)
	// }}
			goto out;
	}

// Apex {{
	RdpClientEntry(&clientEntryPoints);

	context = freerdp_client_context_new(&clientEntryPoints);
	if (!context)
	{
		MessageBox(NULL, _T("无法启动，内部错误！"), _T("提示"), MB_OK | MB_ICONERROR);
		goto out;
		//		return -1;
	}
// }}

	settings = context->settings;
	wfc = (wfContext*)context;
	if (!settings || !wfc)

// Apex {{
	{
		MessageBox(NULL, _T("设置错误！"), _T("错误"), MB_OK | MB_ICONERROR);
		goto out;
	}
// }}

	status = freerdp_client_settings_parse_command_line(settings, argc, argv, FALSE);
	if (status)

// Apex {{
	{
		MessageBox(NULL, _T("解析命令行错误！"), _T("错误"), MB_OK | MB_ICONERROR);
		freerdp_client_settings_command_line_status_print(settings, status, argc, argv);
		goto out;
	}

	// wfreerdp.exe "http://127.0.0.1:7190" 256 "tp_1491560510_ca67fceb75a78c9d" "000000256-admin-administrator-218.244.140.14-20171209-020047"
	//if (strlen(wfc->downloader.filename_base) > 7 && 0 == memcmp(wfc->downloader.filename_base, "http://", 7))
	if (argc == 5)
	{
		strcpy(wfc->downloader.url_base, argv[1]);
		strcpy(wfc->downloader.record_id, argv[2]);
		strcpy(wfc->downloader.session_id, argv[3]);

		GetModuleFileNameA(NULL, wfc->downloader.filename_base, 1023);
		char* szFNtmp = strrchr(wfc->downloader.filename_base, '\\');
		szFNtmp[0] = '\0';
		strcat(wfc->downloader.filename_base, "\\record");
		if (!PathFileExistsA(wfc->downloader.filename_base))
		{
			CreateDirectoryA(wfc->downloader.filename_base, NULL);
		}
		strcat(wfc->downloader.filename_base, "\\");
		strcat(wfc->downloader.filename_base, argv[4]);
		if (!PathFileExistsA(wfc->downloader.filename_base))
		{
			CreateDirectoryA(wfc->downloader.filename_base, NULL);
		}
	}
	else if (argc == 2)
	{
		strcpy(wfc->downloader.filename_base, argv[1]);

		// if give me a folder, it should be the base-name, otherwise, get the path of filename as base-name
		if (!PathFileExistsA(wfc->downloader.filename_base))
		{
			MessageBox(NULL, _T("指定的文件或目录不存在！"), _T("错误"), MB_OK | MB_ICONERROR);
			goto out;
		}

		if (PathIsDirectoryA(wfc->downloader.filename_base))
		{
			if (wfc->downloader.filename_base[strlen(wfc->downloader.filename_base) - 1] == '\\')
				wfc->downloader.filename_base[strlen(wfc->downloader.filename_base) - 1] = '\0';
		}
		else
		{
			char* szTmp = strrchr(wfc->downloader.filename_base, '\\');
			if (NULL == szTmp)
				goto out;
			szTmp[0] = '\0';
		}
	}
	else
	{
		MessageBox(NULL, _T("不太清楚您要做什么啊！"), _T("提示"), MB_OK | MB_ICONQUESTION);
		goto out;
	}

	// create a thread to read files (download missed files) and wait until package-header file exists.
	wfc->record_hdr_exist_event = CreateEventA(NULL, FALSE, FALSE, NULL);
	if (!wf_create_downloader(wfc))
		goto out;
	WaitForSingleObject(wfc->record_hdr_exist_event, INFINITE);


		//freerdp_play_thread_start(wfc);
	if(0 == wfc->record_hdr.basic.width || 0 == wfc->record_hdr.basic.height)
	{
		//MessageBox(NULL, _T("文件不是 Teleport RDP 录像文件格式！"), _T("错误"), MB_OK | MB_ICONERROR);
		goto out;
	}

	wfc->settings->DesktopWidth = wfc->record_hdr.basic.width;
	wfc->settings->DesktopHeight = wfc->record_hdr.basic.height;

	if (wfc->record_hdr.basic.rdp_security == 0)	// 0 = RDP, 1 = SSL
		wfc->settings->UseRdpSecurityLayer = TRUE;
// }}


	if (freerdp_client_start(context) != 0)
		goto out;

	thread = freerdp_client_get_thread(context);
	if (thread)
	{
		if (WaitForSingleObject(thread, INFINITE) == WAIT_OBJECT_0)
		{
			GetExitCodeThread(thread, &dwExitCode);
			ret = dwExitCode;
		}
	}

	if (freerdp_client_stop(context) != 0)
		goto out;

out:

// Apex {{
	if (NULL != wfc->record_hdr_exist_event)
		CloseHandle(wfc->record_hdr_exist_event);
// }}

	freerdp_client_context_free(context);

	if (argv)
	{
		for (i = 0; i < argc; i++)
			free(argv[i]);

		free(argv);
	}
	LocalFree(args);

	return ret;
}
