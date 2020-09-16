#include "bucket_config.h"
#define RAPIDJSON_HAS_STDSTRING 1
#include <rapidjson/document.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/stringbuffer.h>
#include "backend/bitbucket/bitbucket_config.h"

const UString& CBucketConfig::s_sConfigPath = CBucketConfig::makeConfigPath();

vector<CBucketConfig::SRegInfo>& CBucketConfig::s_vRegistry = CBucketConfig::initRegistry();

CBucketConfig::SCommonConfig::SCommonConfig()
	: PanType(kPanTypeNone)
{
}

CBucketConfig::SCommonConfig::~SCommonConfig()
{
}

bool CBucketConfig::SCommonConfig::SetValue(const string& a_sKey, const string& a_sValue)
{
	if (a_sKey == "name")
	{
		Name = a_sValue;
		return true;
	}
	else if (a_sKey == "type")
	{
		EPanType ePanType = getPanType(a_sValue);
		if (ePanType == kPanTypeNone)
		{
			return false;
		}
		PanType = ePanType;
		return true;
	}
	else if (a_sKey == "git_user_name")
	{
		GitConfig.UserName = a_sValue;
		return true;
	}
	else if (a_sKey == "git_user_email")
	{
		GitConfig.UserEMail = a_sValue;
		return true;
	}
	return false;
}

string CBucketConfig::SCommonConfig::GetValue(const string& a_sKey) const
{
	if (a_sKey == "name")
	{
		return Name;
	}
	else if (a_sKey == "type")
	{
		return GetPanTypeName(PanType);
	}
	else if (a_sKey == "git_user_name")
	{
		return GitConfig.UserName;
	}
	else if (a_sKey == "git_user_email")
	{
		return GitConfig.UserEMail;
	}
	return "";
}

vector<string> CBucketConfig::SCommonConfig::GetKeyList() const
{
	vector<string> vKeyList;
	vKeyList.push_back("type");
	GetKeyList(vKeyList);
	vKeyList.push_back("git_user_name");
	vKeyList.push_back("git_user_email");
	return vKeyList;
}

CBucketConfig::STypedValue::STypedValue()
	: ValueType(kValueTypeString)
{
}

string CBucketConfig::STypedValue::GetValueTypeName() const
{
	switch (ValueType)
	{
	case kValueTypeString:
		return "string";
	default:
		return "unknown";
	}
}

string CBucketConfig::STypedValue::GetValueAsString() const
{
	switch (ValueType)
	{
	case kValueTypeString:
		return Value.StringValue;
	default:
		return "";
	}
}

bool CBucketConfig::SOptionExample::Compare(const SOptionExample& lhs, const SOptionExample& rhs)
{
	return lhs.Help < rhs.Help;
}

CBucketConfig::SOption::SOption()
	: Required(false)
{
}

const string& CBucketConfig::SRegInfo::FileName() const
{
	static string c_sName = Replace(Name, " ", "");
	return c_sName;
}

CBucketConfig::CBucketConfig()
{
}

CBucketConfig::~CBucketConfig()
{
	for (vector<SCommonConfig*>::iterator it = m_vConfig.begin(); it != m_vConfig.end(); ++it)
	{
		SCommonConfig* pConfig = *it;
		delete pConfig;
	}
}

const string& CBucketConfig::GetPanTypeName(EPanType a_ePanType)
{
	static const string c_sPanTypeName[kPanTypeCount] =
	{
		"none",
		"bitbucket"
	};
	return c_sPanTypeName[a_ePanType];
}

bool CBucketConfig::EditConfig()
{
	bool bResult = true;
	do
	{
		bResult = loadConfig();
		if (!bResult)
		{
			break;
		}
		while (true)
		{
			bool bHaveRemotes = !m_mConfigNameIndex.empty();
			vector<pair<n32, const char*>> vCommands;
			if (bHaveRemotes)
			{
				printf("Current remotes:\n\n");
				showRemotes();
				printf("\n");
				vCommands.push_back(make_pair('e', "Edit existing remote"));
				vCommands.push_back(make_pair('n', "New remote"));
				vCommands.push_back(make_pair('d', "Delete remote"));
				vCommands.push_back(make_pair('r', "Rename remote"));
				vCommands.push_back(make_pair('c', "Copy remote"));
				vCommands.push_back(make_pair('q', "Quit config"));
			}
			else
			{
				printf("No remotes found - make a new one\n");
				vCommands.push_back(make_pair('n', "New remote"));
				vCommands.push_back(make_pair('q', "Quit config"));
			}
			n32 nCommand = command(vCommands);
			switch (nCommand)
			{
			case 'e':
				{
					string sName = chooseRomote();
					const SRegInfo* pRegInfo = mustFindByName(sName);
					editRemote(pRegInfo, sName);
				}
				break;
			case 'n':
				newRemote(newRemoteName());
				break;
			case 'd':
				deleteRemote(chooseRomote());
				break;
			case 'r':
				renameRemote(chooseRomote());
				break;
			case 'c':
				copyRemote(chooseRomote());
				break;
			case 'q':
				return bResult;
			default:
				break;
			}
		}
	} while (false);
	return bResult;
}

const UString& CBucketConfig::makeConfigPath()
{
	static const UString sConfigPath = UGetModuleDirName() + USTR("/config.json");
	return sConfigPath;
}

vector<CBucketConfig::SRegInfo>& CBucketConfig::initRegistry()
{
	static vector<SRegInfo> c_vRegistry;
	c_vRegistry.push_back(SBitbucketConfig::GetRegInfo());
	return c_vRegistry;
}

CBucketConfig::EPanType CBucketConfig::getPanType(const string& a_sPanTypeName)
{
	for (n32 i = kPanTypeNone; i < kPanTypeCount; i++)
	{
		EPanType ePanType = static_cast<EPanType>(i);
		if (a_sPanTypeName == GetPanTypeName(ePanType))
		{
			return ePanType;
		}
	}
	return kPanTypeNone;
}

const CBucketConfig::SRegInfo* CBucketConfig::find(const string& a_sName)
{
	for (vector<SRegInfo>::const_iterator it = s_vRegistry.begin(); it != s_vRegistry.end(); ++it)
	{
		const SRegInfo& regInfo = *it;
		if (regInfo.Name == a_sName || regInfo.FileName() == a_sName)
		{
			return &regInfo;
		}
	}
	return nullptr;
}

const CBucketConfig::SRegInfo* CBucketConfig::mustFind(const string& a_sName)
{
	const SRegInfo* pRegInfo = find(a_sName);
	if (pRegInfo == nullptr)
	{
		printf("Failed to find remote: didn't find backend called \"%s\"\n", a_sName.c_str());
	}
	return pRegInfo;
}

string CBucketConfig::readLine()
{
	wstring sLineW;
	wchar_t c = L'\0';
	while ((c = getwchar()) != L'\n')
	{
		sLineW.push_back(c);
	}
	wstring::size_type uPos = sLineW.find_first_not_of(L"\t\n\v\f\r \x85\xA0");
	if (uPos != wstring::npos)
	{
		sLineW.erase(0, uPos);
	}
	uPos = sLineW.find_last_not_of(L"\t\n\v\f\r \x85\xA0");
	if (uPos != wstring::npos)
	{
		sLineW.erase(uPos + 1);
	}
	string sLine = WToU8(sLineW);
	return sLine;
}

char CBucketConfig::commandDefault(const vector<pair<n32, const char*>>& a_vCommands, n32 a_nDefaultIndex)
{
	set<n32> sOpts;
	string sOptHelp;
	for (n32 i = 0; i < static_cast<n32>(a_vCommands.size()); i++)
	{
		const pair<n32, const char*>& command = a_vCommands[i];
		const char* pDef = "";
		if (i == a_nDefaultIndex)
		{
			pDef = " (default)";
		}
		printf("%c) %s%s\n", static_cast<char>(command.first), command.second, pDef);
		sOpts.insert(command.first);
		if (i != 0)
		{
			sOptHelp += "/";
		}
		sOptHelp.append(1, static_cast<char>(command.first));
	}
	while (true)
	{
		printf("%s> ", sOptHelp.c_str());
		string sResult = readLine();
		transform(sResult.begin(), sResult.end(), sResult.begin(), ::tolower);
		if (sResult.empty() && a_nDefaultIndex >= 0 && a_nDefaultIndex < static_cast<n32>(a_vCommands.size()))
		{
			return a_vCommands[a_nDefaultIndex].first;
		}
		if (sResult.size() != 1)
		{
			continue;
		}
		set<n32>::const_iterator it = sOpts.find(sResult[0]);
		if (it != sOpts.end())
		{
			return sResult[0];
		}
	}
}

char CBucketConfig::command(const vector<pair<n32, const char*>>& a_vCommands)
{
	return commandDefault(a_vCommands, -1);
}

bool CBucketConfig::confirm(bool a_bDefault)
{
	n32 nDefaultIndex = 0;
	if (!a_bDefault)
	{
		nDefaultIndex = 1;
	}
	vector<pair<n32, const char*>> vCommands;
	vCommands.push_back(make_pair('y', "Yes"));
	vCommands.push_back(make_pair('n', "No"));
	return commandDefault(vCommands, nDefaultIndex) == 'y';
}

string CBucketConfig::choose(const string& a_sWhat, const vector<string>& a_sDefaults, const vector<string>& a_vHelp, bool a_bNewOk)
{
	const char* pValueDescription = "an existing";
	if (a_bNewOk)
	{
		pValueDescription = "your own";
	}
	printf("Choose a number from below, or type in %s value\n", pValueDescription);
	for (n32 i = 0; i < static_cast<n32>(a_sDefaults.size()); i++)
	{
		const string& sText = a_sDefaults[i];
		vector<string> vLines;
		if (i < static_cast<n32>(a_vHelp.size()) && !a_vHelp[i].empty())
		{
			vector<string> vParts = Split(a_vHelp[i], "\n");
			vLines.insert(vLines.end(), vParts.begin(), vParts.end());
		}
		vLines.push_back("\"" + sText + "\"");
		n32 nPos = i + 1;
		if (vLines.size() == 1)
		{
			printf("%2d > %s\n", nPos, sText.c_str());
		}
		else
		{
			n32 nMid = static_cast<n32>((vLines.size() - 1) / 2);
			for (n32 j = 0; j < static_cast<n32>(vLines.size()); j++)
			{
				const string& sLine = vLines[j];
				char nSep = '|';
				if (j == 0)
				{
					nSep = '/';
				}
				else if (j == static_cast<n32>(vLines.size()) - 1)
				{
					nSep = '\\';
				}
				string sNumber = "  ";
				if (j == nMid)
				{
					sNumber = Format("%2d", nPos);
				}
				printf("%s %c %s\n", sNumber.c_str(), nSep, sLine.c_str());
			}
		}
	}
	while (true)
	{
		printf("%s> ", a_sWhat.c_str());
		string sResult = readLine();
		if (sResult.empty() || sResult.find_first_not_of("0123456789") != string::npos)
		{
			if (a_bNewOk)
			{
				return sResult;
			}
			for (vector<string>::const_iterator it = a_sDefaults.begin(); it != a_sDefaults.end(); ++it)
			{
				if (sResult == *it)
				{
					return sResult;
				}
			}
		}
		else
		{
			n32 nPos = SToN32(sResult);
			if (nPos >= 1 && nPos <= static_cast<n32>(a_sDefaults.size()))
			{
				return a_sDefaults[nPos - 1];
			}
		}
	}
}

string CBucketConfig::chooseOption(const SOption& a_Option, const string& a_sName)
{
	printf("%s\n", a_Option.Help.c_str());
	string sWhat = Format("%s value", a_Option.Default.GetValueTypeName().c_str());
	string sInput;
	while (true)
	{
		printf("Enter a %s. Press Enter for the default (\"%s\").\n", sWhat.c_str(), a_Option.Default.GetValueAsString().c_str());
		if (!a_Option.Examples.empty())
		{
			vector<string> vValues;
			vector<string> vHelp;
			for (vector<SOptionExample>::const_iterator it = a_Option.Examples.begin(); it != a_Option.Examples.end(); ++it)
			{
				const SOptionExample& optionExample = *it;
				vValues.push_back(optionExample.Value);
				vHelp.push_back(optionExample.Help);
			}
			sInput = choose(a_Option.Name, vValues, vHelp, true);
		}
		else
		{
			printf("%s> ", a_Option.Name.c_str());
			sInput = readLine();
		}
		if (sInput.empty())
		{
			if (a_Option.Required && a_Option.Default.GetValueAsString().empty())
			{
				printf("This value is required and it has no default.\n");
				continue;
			}
			break;
		}
		break;
	}
	return sInput;
}

bool CBucketConfig::checkConfigName(const string& a_sConfigName)
{
	static regex rConfigName("^[\\w_ -]+$", regex_constants::ECMAScript);
	if (!regex_match(a_sConfigName, rConfigName))
	{
		return false;
	}
	if (StartWith(a_sConfigName, "-"))
	{
		return false;
	}
	return true;
}

bool CBucketConfig::isDriveLetter(const string& a_sName)
{
#if SDW_PLATFORM == SDW_PLATFORM_WINDOWS
	if (a_sName.size() != 1)
	{
		return false;
	}
	char nName = a_sName[0];
	return (nName >= 'A' && nName <= 'Z') || (nName >= 'a' && nName <= 'z');
#else
	return false;
#endif
}

CBucketConfig::SOption CBucketConfig::fsOption()
{
	SOption option;
	option.Name = "Storage";
	option.Help = "Type of storage to configure.";
	option.Default.ValueType = STypedValue::kValueTypeString;
	option.Default.Value.StringValue = "";
	for (vector<SRegInfo>::const_iterator it = s_vRegistry.begin(); it != s_vRegistry.end(); ++it)
	{
		const SRegInfo& regInfo = *it;
		SOptionExample example;
		example.Value = regInfo.Name;
		example.Help = regInfo.Description;
		option.Examples.push_back(example);
	}
	sort(option.Examples.begin(), option.Examples.end(), SOptionExample::Compare);
	return option;
}

bool CBucketConfig::saveConfig() const
{
	RAPIDJSON_NAMESPACE::Document configDoc;
	configDoc.SetObject();
	for (vector<SCommonConfig*>::const_iterator it = m_vConfig.begin(); it != m_vConfig.end(); ++it)
	{
		const SCommonConfig* pConfig = *it;
		if (!pConfig->Save(&configDoc, &configDoc.GetAllocator()))
		{
			return false;
		}
	}
	RAPIDJSON_NAMESPACE::StringBuffer stringBuffer;
	RAPIDJSON_NAMESPACE::PrettyWriter<RAPIDJSON_NAMESPACE::StringBuffer> writer(stringBuffer);
	configDoc.Accept(writer);
	FILE* fp = UFopen(s_sConfigPath.c_str(), USTR("wb"));
	if (fp == nullptr)
	{
		return false;
	}
	fwrite(stringBuffer.GetString(), 1, stringBuffer.GetSize(), fp);
	fclose(fp);
	return true;
}

bool CBucketConfig::loadConfig()
{
	string sConfig;
	FILE* fp = UFopen(s_sConfigPath.c_str(), USTR("rb"));
	if (fp == nullptr)
	{
		fp = UFopen(s_sConfigPath.c_str(), USTR("wb"));
		if (fp == nullptr)
		{
			return false;
		}
	}
	else
	{
		fseek(fp, 0, SEEK_END);
		u32 uConfigSize = ftell(fp);
		fseek(fp, 0, SEEK_SET);
		char* pConfig = new char[uConfigSize + 1];
		fread(pConfig, 1, uConfigSize, fp);
		pConfig[uConfigSize] = '\0';
		sConfig = pConfig;
		delete[] pConfig;
	}
	fclose(fp);
	RAPIDJSON_NAMESPACE::Document configDoc;
	if (sConfig.empty())
	{
		configDoc.SetObject();
	}
	else
	{
		configDoc.Parse(sConfig.c_str());
	}
	if (!configDoc.IsObject())
	{
		return false;
	}
	m_mConfigNameIndex.clear();
	for (RAPIDJSON_NAMESPACE::Value::ConstMemberIterator it = configDoc.MemberBegin(); it != configDoc.MemberEnd(); ++it)
	{
		const RAPIDJSON_NAMESPACE::Value& name = it->name;
		if (!name.IsString())
		{
			return false;
		}
		string sName = name.GetString();
		const RAPIDJSON_NAMESPACE::Value& value = it->value;
		if (!value.IsObject())
		{
			return false;
		}
		if (!value.HasMember("type") || !value["type"].IsString())
		{
			return false;
		}
		EPanType ePanType = kPanTypeNone;
		string sPanTypeName = value["type"].GetString();
		if (sPanTypeName == GetPanTypeName(kPanTypeBitbucket))
		{
			ePanType = kPanTypeBitbucket;
		}
		switch (ePanType)
		{
		case kPanTypeBitbucket:
			{
				SBitbucketConfig* pConfig = new SBitbucketConfig;
				if (!pConfig->Load(&value, sName))
				{
					delete pConfig;
					return false;
				}
				if (!m_mConfigNameIndex.insert(make_pair(sName, static_cast<n32>(m_vConfig.size()))).second)
				{
					delete pConfig;
					return false;
				}
				m_vConfig.push_back(pConfig);
			}
			break;
		default:
			return false;
		}
	}
	return true;
}

void CBucketConfig::refreshIndex()
{
	m_mConfigNameIndex.clear();
	for (n32 i = 0; i < static_cast<n32>(m_vConfig.size()); i++)
	{
		const SCommonConfig* pConfig = m_vConfig[i];
		m_mConfigNameIndex.insert(make_pair(pConfig->Name, i));
	}
}

bool CBucketConfig::setValue(const string& a_sSection, const string& a_sKey, const string& a_sValue)
{
	map<string, n32>::const_iterator it = m_mConfigNameIndex.find(a_sSection);
	if (it != m_mConfigNameIndex.end())
	{
		SCommonConfig* pConfig = m_vConfig[it->second];
		if (a_sKey == "type" && a_sValue != pConfig->GetValue(a_sKey))
		{
			return false;
		}
		return pConfig->SetValue(a_sKey, a_sValue);
	}
	if (a_sKey != "type")
	{
		return false;
	}
	SCommonConfig* pConfig = nullptr;
	EPanType ePanType = getPanType(a_sValue);
	switch (ePanType)
	{
	case kPanTypeBitbucket:
		pConfig = new SBitbucketConfig;
		break;
	default:
		return false;
	}
	if (!pConfig->SetValue("name", a_sSection))
	{
		delete pConfig;
		return false;
	}
	if (!pConfig->SetValue("type", a_sValue))
	{
		delete pConfig;
		return false;
	}
	m_vConfig.push_back(pConfig);
	refreshIndex();
	return true;
}

void CBucketConfig::fileSet(const string& a_sSection, const string& a_sKey, const string& a_sValue)
{
	map<string, n32>::const_iterator it = m_mConfigNameIndex.find(a_sSection);
	if (it != m_mConfigNameIndex.end())
	{
		SCommonConfig* pConfig = m_vConfig[it->second];
		pConfig->SetValue(a_sKey, a_sValue);
	}
}

string CBucketConfig::mustValue(const string& a_sSection, const string& a_sKey, const string& a_sDefaultValue /* = "" */) const
{
	string sValue = a_sDefaultValue;
	map<string, n32>::const_iterator it = m_mConfigNameIndex.find(a_sSection);
	if (it != m_mConfigNameIndex.end())
	{
		const SCommonConfig* pConfig = m_vConfig[it->second];
		sValue = pConfig->GetValue(a_sKey);
		if (sValue.empty())
		{
			sValue = a_sDefaultValue;
		}
	}
	return sValue;
}

string CBucketConfig::fileGet(const string& a_sSection, const string& a_sKey, const string& a_sDefaultValue /* = "" */) const
{
	return mustValue(a_sSection, a_sKey, a_sDefaultValue);
}

void CBucketConfig::showRemotes() const
{
	if (m_mConfigNameIndex.empty())
	{
		return;
	}
	printf("%-20s %s\n", "Name", "Type");
	printf("%-20s %s\n", "====", "====");
	for (map<string, n32>::const_iterator it = m_mConfigNameIndex.begin(); it != m_mConfigNameIndex.end(); ++it)
	{
		const SCommonConfig* pConfig = m_vConfig[it->second];
		printf("%-20s %s\n", pConfig->Name.c_str(), fileGet(pConfig->Name, "type").c_str());
	}
}

vector<string> CBucketConfig::getSectionList() const
{
	vector<string> vRemotes;
	for (map<string, n32>::const_iterator it = m_mConfigNameIndex.begin(); it != m_mConfigNameIndex.end(); ++it)
	{
		vRemotes.push_back(it->first);
	}
	return vRemotes;
}

string CBucketConfig::chooseRomote() const
{
	vector<string> vRemotes(getSectionList());
	vector<string> vHelp;
	return choose("remote", vRemotes, vHelp, false);
}

const CBucketConfig::SRegInfo* CBucketConfig::mustFindByName(const string& a_sName) const
{
	string sPanTypeName = fileGet(a_sName, "type");
	if (sPanTypeName.empty() || sPanTypeName == GetPanTypeName(kPanTypeNone))
	{
		printf("Couldn't find type of fs for \"%s\"\n", sPanTypeName.c_str());
	}
	return mustFind(sPanTypeName);
}

void CBucketConfig::showRemote(const string& a_sName) const
{
	printf("--------------------\n");
	printf("[%s]\n", a_sName.c_str());
	RAPIDJSON_NAMESPACE::Document configDoc;
	configDoc.SetObject();
	map<string, n32>::const_iterator it = m_mConfigNameIndex.find(a_sName);
	if (it != m_mConfigNameIndex.end())
	{
		const SCommonConfig* pConfig = m_vConfig[it->second];
		vector<string> vKeyList = pConfig->GetKeyList();
		for (vector<string>::const_iterator itKey = vKeyList.begin(); itKey != vKeyList.end(); ++itKey)
		{
			const string& sKey = *itKey;
			string sValue = mustValue(a_sName, sKey, "");
			printf("%s = %s\n", sKey.c_str(), sValue.c_str());
		}
		if (!pConfig->Save(&configDoc, &configDoc.GetAllocator()))
		{
			configDoc.SetObject();
		}
	}
	RAPIDJSON_NAMESPACE::StringBuffer stringBuffer;
	RAPIDJSON_NAMESPACE::PrettyWriter<RAPIDJSON_NAMESPACE::StringBuffer> writer(stringBuffer);
	configDoc.Accept(writer);
	printf("\n%s\n", stringBuffer.GetString());
	printf("--------------------\n");
}

void CBucketConfig::editOptions(const SRegInfo* a_pRegInfo, const string& a_sName, bool a_bIsNew)
{
	if (a_pRegInfo != nullptr)
	{
		const SRegInfo& regInfo = *a_pRegInfo;
		for (vector<SOption>::const_iterator it = regInfo.Options.begin(); it != regInfo.Options.end(); ++it)
		{
			const SOption& option = *it;
			if (!a_bIsNew)
			{
				printf("Value \"%s\" = \"%s\"\n", option.Name.c_str(), fileGet(a_sName, option.Name).c_str());
				printf("Edit? (y/n)>\n");
				if (!confirm(false))
				{
					continue;
				}
			}
			fileSet(a_sName, option.Name, chooseOption(option, a_sName));
		}
	}
}

void CBucketConfig::deleteSection(const string& a_sName)
{
	map<string, n32>::const_iterator it = m_mConfigNameIndex.find(a_sName);
	if (it == m_mConfigNameIndex.end())
	{
		return;
	}
	n32 nIndex = it->second;
	SCommonConfig* pConfig = m_vConfig[nIndex];
	delete pConfig;
	m_vConfig.erase(m_vConfig.begin() + nIndex);
	refreshIndex();
}

bool CBucketConfig::okRemote(const string& a_sName)
{
	showRemote(a_sName);
	vector<pair<n32, const char*>> vCommands;
	vCommands.push_back(make_pair('y', "Yes this is OK"));
	vCommands.push_back(make_pair('e', "Edit this remote"));
	vCommands.push_back(make_pair('d', "Delete this remote"));
	n32 nCommand = commandDefault(vCommands, 0);
	switch (nCommand)
	{
	case 'y':
		return true;
	case 'e':
		return false;
	case 'd':
		deleteSection(a_sName);
		return true;
	default:
		printf("Bad choice %c\n", static_cast<char>(nCommand));
		break;
	}
	return false;
}

void CBucketConfig::editRemote(const SRegInfo* a_pRegInfo, const string& a_sName)
{
	showRemote(a_sName);
	printf("Edit remote\n");
	while (true)
	{
		editOptions(a_pRegInfo, a_sName, false);
		if (okRemote(a_sName))
		{
			break;
		}
	}
	saveConfig();
	// TODO
}

string CBucketConfig::newRemoteName() const
{
	while (true)
	{
		printf("name> ");
		string sName = readLine();
		map<string, n32>::const_iterator it = m_mConfigNameIndex.find(sName);
		if (it != m_mConfigNameIndex.end())
		{
			printf("Remote \"%s\" already exists.\n", sName.c_str());
			continue;
		}
		bool bValid = checkConfigName(sName);
		if (sName.empty())
		{
			printf("Can't use empty name.\n");
		}
		else if (isDriveLetter(sName))
		{
			printf("Can't use \"%s\" as it can be confused with a drive letter.\n", sName.c_str());
		}
		else if (!bValid)
		{
			printf("Can't use \"%s\".\n", sName.c_str());
		}
		else
		{
			return sName;
		}
	}
}

void CBucketConfig::newRemote(const string& a_sName)
{
	string sNewType;
	const SRegInfo* pRegInfo = nullptr;
	while (true)
	{
		sNewType = chooseOption(fsOption(), a_sName);
		pRegInfo = find(sNewType);
		if (pRegInfo == nullptr)
		{
			printf("Bad remote \"%s\": didn't find backend called \"%s\"\n", sNewType.c_str(), sNewType.c_str());
			continue;
		}
		break;
	}
	setValue(a_sName, "type", sNewType);
	editOptions(pRegInfo, a_sName, true);
	// TODO
	if (okRemote(a_sName))
	{
		saveConfig();
		return;
	}
	editRemote(pRegInfo, a_sName);
}

void CBucketConfig::deleteRemote(const string& a_sName)
{
	deleteSection(a_sName);
	saveConfig();
}

string CBucketConfig::copyRemoteInternal(const string& a_sName)
{
	string sNewName = newRemoteName();
	map<string, n32>::const_iterator it = m_mConfigNameIndex.find(a_sName);
	if (it != m_mConfigNameIndex.end())
	{
		const SCommonConfig* pConfig = m_vConfig[it->second];
		vector<string> vKeyList = pConfig->GetKeyList();
		for (vector<string>::const_iterator itKey = vKeyList.begin(); itKey != vKeyList.end(); ++itKey)
		{
			const string& sKey = *itKey;
			string sValue = mustValue(a_sName, sKey, "");
			setValue(sNewName, sKey, sValue);
		}
	}
	return sNewName;
}

void CBucketConfig::renameRemote(const string& a_sName)
{
	printf("Enter new name for \"%s\" remote.\n", a_sName.c_str());
	string sNameName = copyRemoteInternal(a_sName);
	if (a_sName != sNameName)
	{
		deleteSection(a_sName);
		saveConfig();
	}
}

void CBucketConfig::copyRemote(const string& a_sName)
{
	printf("Enter name for copy of \"%s\" remote.\n", a_sName.c_str());
	copyRemoteInternal(a_sName);
	saveConfig();
}
