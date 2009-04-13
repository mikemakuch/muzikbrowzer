#ifndef _MUZIKBROWZER__VERSION__H__
#define _MUZIKBROWZER__VERSION__H__
static const char MUZIKBROWZER_VERSION[] = "2.2.0 2009-04-13 16:14:49";

#include "MyString.h"
#include "MBConfig.h"

class MBVersion {
public:
	MBVersion(MBConfig & config):m_Config(config),
		m_talkedToMama(FALSE) {}
	~MBVersion(){}
	CString MBVERSION() {
		return CString(MUZIKBROWZER_VERSION);
	}

	unsigned int thisVersion();
	unsigned int publicVersion();
	BOOL phoneHome();
	BOOL needUpdate();
	BOOL goodLicense();
protected:
	unsigned int s2i(const CString & s);
	BOOL timeToPhoneHome();
	MyHash m_pairs;
	BOOL m_talkedToMama;
	CString m_license;
	MBConfig & m_Config;
};

#endif
