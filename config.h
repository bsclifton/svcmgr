// config.h
// brian clifton 2003

#ifndef CONFIG_H
#define CONFIG_H

#include <stdio.h>
#include <string.h>
#include <windows.h>

typedef unsigned char byte;

class Config {
	//members
	public:
		static int PortNumber;
		static int MaxConnections;
		static wchar_t PaymentServiceHost[256];
		static wchar_t PaymentServiceSite[1024];
		static wchar_t PaymentServicePage[1024];
		static wchar_t ConnectionString[1024];
		static bool WriteToLog;

	//methods
	public:
		static bool Load(void);
		static bool Save(void);
};

#endif