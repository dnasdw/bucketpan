#ifndef BUCKET_CONFIG_H_
#define BUCKET_CONFIG_H_

#include <sdw.h>

class CBucketConfig
{
public:
	enum EPanType
	{
		kPanTypeNone,
		kPanTypeBitbucket,
		kPanTypeCount
	};
	struct SGitConfig
	{
		string UserName;
		string UserEMail;
	};
	struct SCommonConfig
	{
		string Name;
		EPanType PanType;
		SGitConfig GitConfig;
		SCommonConfig();
		virtual ~SCommonConfig();
		virtual bool Save(void* a_pConfigJSONObject, void* a_pAllocator) const SDW_PURE;
		virtual bool Load(const void* a_pConfigJSONObject, const string& a_sName) SDW_PURE;
		virtual bool SetValue(const string& a_sKey, const string& a_sValue);
		virtual string GetValue(const string& a_sKey) const;
		virtual void GetKeyList(vector<string>& a_vKeyList) const SDW_PURE;
		vector<string> GetKeyList() const;
	};
	struct STypedValue
	{
		enum EValueType
		{
			kValueTypeString
		} ValueType;
		struct SValue
		{
			string StringValue;
		} Value;
		STypedValue();
		string GetValueTypeName() const;
		string GetValueAsString() const;
	};
	struct SOptionExample
	{
		string Value;
		string Help;
		static bool Compare(const SOptionExample& lhs, const SOptionExample& rhs);
	};
	struct SOption
	{
		string Name;
		string Help;
		STypedValue Default;
		vector<SOptionExample> Examples;
		bool Required;
		SOption();
	};
	struct SRegInfo
	{
		string Name;
		string Description;
		vector<SOption> Options;
		const string& FileName() const;
	};
	CBucketConfig();
	~CBucketConfig();
	static const string& GetPanTypeName(EPanType a_ePanType);
	bool EditConfig();
private:
	static const UString& makeConfigPath();
	static vector<SRegInfo>& initRegistry();
	static EPanType getPanType(const string& a_sPanTypeName);
	static const SRegInfo* find(const string& a_sName);
	static const SRegInfo* mustFind(const string& a_sName);
	static string readLine();
	static char commandDefault(const vector<pair<n32, const char*>>& a_vCommands, n32 a_nDefaultIndex);
	static char command(const vector<pair<n32, const char*>>& a_vCommands);
	static bool confirm(bool a_bDefault);
	static string choose(const string& a_sWhat, const vector<string>& a_sDefaults, const vector<string>& a_vHelp, bool a_bNewOk);
	static string chooseOption(const SOption& a_Option, const string& a_sName);
	static bool checkConfigName(const string& a_sConfigName);
	static bool isDriveLetter(const string& a_sName);
	static SOption fsOption();
	bool saveConfig() const;
	bool loadConfig();
	void refreshIndex();
	bool setValue(const string& a_sSection, const string& a_sKey, const string& a_sValue);
	void fileSet(const string& a_sSection, const string& a_sKey, const string& a_sValue);
	string mustValue(const string& a_sSection, const string& a_sKey, const string& a_sDefaultValue = "") const;
	string fileGet(const string& a_sSection, const string& a_sKey, const string& a_sDefaultValue = "") const;
	void showRemotes() const;
	vector<string> getSectionList() const;
	string chooseRomote() const;
	const SRegInfo* mustFindByName(const string& a_sName) const;
	void showRemote(const string& a_sName) const;
	void editOptions(const SRegInfo* a_pRegInfo, const string& a_sName, bool a_bIsNew);
	void deleteSection(const string& a_sName);
	bool okRemote(const string& a_sName);
	void editRemote(const SRegInfo* a_pRegInfo, const string& a_sName);
	string newRemoteName() const;
	void newRemote(const string& a_sName);
	void deleteRemote(const string& a_sName);
	string copyRemoteInternal(const string& a_sName);
	void renameRemote(const string& a_sName);
	void copyRemote(const string& a_sName);
	static const UString& s_sConfigPath;
	static vector<SRegInfo>& s_vRegistry;
	vector<SCommonConfig*> m_vConfig;
	map<string, n32> m_mConfigNameIndex;
};

#endif	// BUCKET_CONFIG_H_
