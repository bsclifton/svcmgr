// config.cpp
// brian clifton 2003

#include "config.h"

int Config::PortNumber = 7777;
int Config::MaxConnections = 1024;
wchar_t Config::PaymentServiceHost[256];
wchar_t Config::PaymentServiceSite[1024];
wchar_t Config::PaymentServicePage[1024];
wchar_t Config::ConnectionString[1024];
bool Config::WriteToLog = false;

#define MY_SERVICE_REGISTRY_PATH L"Software\\CompanyName"
#define MY_SERVICE_REGISTRY_INSTALL_PATH_KEY L"MyServiceInstallPath"
#define CONFIG_FILE_NAME "config.bin"

bool Config::Load(void) {
	char  strPath[4096];
	DWORD valueSize, valueType;
	HKEY  keyPath;

	// get path from registry
	if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, MY_SERVICE_REGISTRY_PATH, 0, KEY_READ, &keyPath))
		return(false);
	RegQueryValueEx(keyPath, MY_SERVICE_REGISTRY_INSTALL_PATH_KEY, 0, &valueType, (byte*)strPath, &valueSize);
	RegCloseKey(keyPath);

	// append name
	strcat_s(strPath, 4096, CONFIG_FILE_NAME);

	// open file
	FILE* fConfig = NULL;
	fopen_s(&fConfig, strPath, "rb");
	if (!fConfig) {
		// if file is not found, set defaults and write
		PortNumber = 7777;
		MaxConnections = 1024;
		wcscpy_s(PaymentServiceHost, 256, L"hostname-goes-here");
		wcscpy_s(PaymentServiceSite, 1024, L"https://hostname-goes-here");
		wcscpy_s(PaymentServicePage, 1024, L"PaymentServicePage.asp");
		wcscpy_s(ConnectionString, 1024, L"Provider=sqloledb;Data Source=DBNAMEHERE;Initial Catalog=DEFAULTDBHERE;uid=sa;pwd=PWDHERE");
		WriteToLog = false;
		return Save();
	}

	// read data
	byte bWriteToLog;
	fread(&PortNumber, 4, 1, fConfig);
	fread(&MaxConnections, 4, 1, fConfig);
	fread(&PaymentServiceHost, 256, 1, fConfig);
	fread(&PaymentServiceSite, 1024, 1, fConfig);
	fread(&PaymentServicePage, 1024, 1, fConfig);
	fread(&ConnectionString, 1024, 1, fConfig);
	fread(&bWriteToLog, 1, 1, fConfig);
	WriteToLog = bWriteToLog ? true : false;

	// close
	fclose(fConfig);
	return true;
}

bool Config::Save(void) {
	char  strPath[4096];
	DWORD valueSize, valueType;
	HKEY  keyPath;

	// get path from registry
	if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, MY_SERVICE_REGISTRY_PATH, 0, KEY_READ, &keyPath))
		return(false);
	RegQueryValueEx(keyPath, MY_SERVICE_REGISTRY_INSTALL_PATH_KEY, 0, &valueType, (byte*)strPath, &valueSize);
	RegCloseKey(keyPath);

	// append name
	strcat_s(strPath, 4096, CONFIG_FILE_NAME);

	// open file
	FILE* fConfig = NULL;
	fopen_s(&fConfig, strPath, "wb");
	if (!fConfig) {
		return false;
	}

	// write data
	fwrite(&PortNumber, 4, 1, fConfig);
	fwrite(&MaxConnections, 4, 1, fConfig);
	fwrite(&PaymentServiceHost, 256, 1, fConfig);
	fwrite(&PaymentServiceSite, 1024, 1, fConfig);
	fwrite(&PaymentServicePage, 1024, 1, fConfig);
	fwrite(&ConnectionString, 1024, 1, fConfig);
	byte bWriteToLog = WriteToLog ? 1 : 0;
	fwrite(&bWriteToLog, 1, 1, fConfig);

	// close
	fclose(fConfig);
	return true;
}
