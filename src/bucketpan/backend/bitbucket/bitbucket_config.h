#ifndef BITBUCKET_CONFIG_H_
#define BITBUCKET_CONFIG_H_

#include "../../bucket_config.h"

struct SBitbucketConfig : public CBucketConfig::SCommonConfig
{
	string UserName;
	string Project;
	string ProjectKey;
	string AppPassword;
	SBitbucketConfig();
	virtual ~SBitbucketConfig();
	static const CBucketConfig::SRegInfo& GetRegInfo();
	virtual bool Save(void* a_pConfigJSONObject, void* a_pAllocator) const;
	virtual bool Load(const void* a_pConfigJSONObject, const string& a_sName);
	virtual bool SetValue(const string& a_sKey, const string& a_sValue);
	virtual string GetValue(const string& a_sKey) const;
	virtual void GetKeyList(vector<string>& a_vKeyList) const;
};

#endif	// BITBUCKET_CONFIG_H_
