#include "bitbucket_config.h"
#define RAPIDJSON_HAS_STDSTRING 1
#include <rapidjson/document.h>

const CBucketConfig::SRegInfo& SBitbucketConfig::GetRegInfo()
{
	static CBucketConfig::SRegInfo c_RegInfo;
	static bool c_bInit = false;
	if (!c_bInit)
	{
		c_RegInfo.Name = CBucketConfig::GetPanTypeName(CBucketConfig::kPanTypeBitbucket);
		c_RegInfo.Description = "bitbucket.org";
		{
			CBucketConfig::SOption option;
			option.Name = "user_name";
			option.Help = "// TODO";
			option.Default.ValueType = CBucketConfig::STypedValue::kValueTypeString;
			option.Default.Value.StringValue = "";
			option.Required = true;
			c_RegInfo.Options.push_back(option);
		}
		{
			CBucketConfig::SOption option;
			option.Name = "project";
			option.Help = "// TODO";
			option.Default.ValueType = CBucketConfig::STypedValue::kValueTypeString;
			option.Default.Value.StringValue = "";
			option.Required = true;
			c_RegInfo.Options.push_back(option);
		}
		{
			CBucketConfig::SOption option;
			option.Name = "project_key";
			option.Help = "// TODO";
			option.Default.ValueType = CBucketConfig::STypedValue::kValueTypeString;
			option.Default.Value.StringValue = "";
			option.Required = true;
			c_RegInfo.Options.push_back(option);
		}
		{
			CBucketConfig::SOption option;
			option.Name = "app_password";
			option.Help = "// TODO";
			option.Default.ValueType = CBucketConfig::STypedValue::kValueTypeString;
			option.Default.Value.StringValue = "";
			option.Required = true;
			c_RegInfo.Options.push_back(option);
		}
		{
			CBucketConfig::SOption option;
			option.Name = "git_user_name";
			option.Help = "// TODO";
			option.Default.ValueType = CBucketConfig::STypedValue::kValueTypeString;
			option.Default.Value.StringValue = "";
			option.Required = true;
			c_RegInfo.Options.push_back(option);
		}
		{
			CBucketConfig::SOption option;
			option.Name = "git_user_email";
			option.Help = "// TODO";
			option.Default.ValueType = CBucketConfig::STypedValue::kValueTypeString;
			option.Default.Value.StringValue = "";
			option.Required = true;
			c_RegInfo.Options.push_back(option);
		}
		c_bInit = true;
	}
	return c_RegInfo;
}

SBitbucketConfig::SBitbucketConfig()
{
}

SBitbucketConfig::~SBitbucketConfig()
{
}

bool SBitbucketConfig::Save(void* a_pConfigJSONObject, void* a_pAllocator) const
{
	if (a_pConfigJSONObject == nullptr)
	{
		return false;
	}
	if (a_pAllocator == nullptr)
	{
		return false;
	}
	RAPIDJSON_NAMESPACE::Value* pConfigParent = reinterpret_cast<RAPIDJSON_NAMESPACE::Value*>(a_pConfigJSONObject);
	if (!pConfigParent->IsObject())
	{
		return false;
	}
	RAPIDJSON_NAMESPACE::Document::AllocatorType& docAllocator = *reinterpret_cast<RAPIDJSON_NAMESPACE::Document::AllocatorType*>(a_pAllocator);
	RAPIDJSON_NAMESPACE::Value name;
	name.SetString(Name, docAllocator);
	RAPIDJSON_NAMESPACE::Value config;
	config.SetObject();
	config.AddMember("type", CBucketConfig::GetPanTypeName(PanType), docAllocator);
	config.AddMember("user_name", UserName, docAllocator);
	config.AddMember("project", Project, docAllocator);
	config.AddMember("project_key", ProjectKey, docAllocator);
	RAPIDJSON_NAMESPACE::Value auth;
	auth.SetObject();
	auth.AddMember("app_password", AppPassword, docAllocator);
	config.AddMember("auth", auth, docAllocator);
	RAPIDJSON_NAMESPACE::Value git;
	git.SetObject();
	git.AddMember("user_name", GitConfig.UserName, docAllocator);
	git.AddMember("user_email", GitConfig.UserEMail, docAllocator);
	config.AddMember("git", git, docAllocator);
	pConfigParent->AddMember(name, config, docAllocator);
	return true;
}

bool SBitbucketConfig::Load(const void* a_pConfigJSONObject, const string& a_sName)
{
	if (a_pConfigJSONObject == nullptr)
	{
		return false;
	}
	const RAPIDJSON_NAMESPACE::Value* pConfig = reinterpret_cast<const RAPIDJSON_NAMESPACE::Value*>(a_pConfigJSONObject);
	if (!pConfig->IsObject())
	{
		return false;
	}
	const RAPIDJSON_NAMESPACE::Value& config = *pConfig;
	Name = a_sName;
	PanType = CBucketConfig::kPanTypeBitbucket;
	if (!config.HasMember("user_name") || !config["user_name"].IsString())
	{
		return false;
	}
	UserName = config["user_name"].GetString();
	if (!config.HasMember("project") || !config["project"].IsString())
	{
		return false;
	}
	Project = config["project"].GetString();
	if (!config.HasMember("project_key") || !config["project_key"].IsString())
	{
		return false;
	}
	ProjectKey = config["project_key"].GetString();
	if (!config.HasMember("auth") || !config["auth"].IsObject() || !config["auth"].HasMember("app_password") || !config["auth"]["app_password"].IsString())
	{
		return false;
	}
	AppPassword = config["auth"]["app_password"].GetString();
	if (!config.HasMember("git") || !config["git"].IsObject())
	{
		return false;
	}
	if (!config["git"].HasMember("user_name") || !config["git"]["user_name"].IsString())
	{
		return false;
	}
	GitConfig.UserName = config["git"]["user_name"].GetString();
	if (!config["git"].HasMember("user_email") || !config["git"]["user_email"].IsString())
	{
		return false;
	}
	GitConfig.UserEMail = config["git"]["user_email"].GetString();
	return true;
}

bool SBitbucketConfig::SetValue(const string& a_sKey, const string& a_sValue)
{
	if (a_sKey == "user_name")
	{
		UserName = a_sValue;
		return true;
	}
	else if (a_sKey == "project")
	{
		Project = a_sValue;
		return true;
	}
	else if (a_sKey == "project_key")
	{
		ProjectKey = a_sValue;
		return true;
	}
	else if (a_sKey == "app_password")
	{
		AppPassword = a_sValue;
		return true;
	}
	return CBucketConfig::SCommonConfig::SetValue(a_sKey, a_sValue);
}

string SBitbucketConfig::GetValue(const string& a_sKey) const
{
	if (a_sKey == "user_name")
	{
		return UserName;
	}
	else if (a_sKey == "project")
	{
		return Project;
	}
	else if (a_sKey == "project_key")
	{
		return ProjectKey;
	}
	else if (a_sKey == "app_password")
	{
		return AppPassword;
	}
	return CBucketConfig::SCommonConfig::GetValue(a_sKey);
}

void SBitbucketConfig::GetKeyList(vector<string>& a_vKeyList) const
{
	a_vKeyList.push_back("user_name");
	a_vKeyList.push_back("project");
	a_vKeyList.push_back("project_key");
	a_vKeyList.push_back("app_password");
}
