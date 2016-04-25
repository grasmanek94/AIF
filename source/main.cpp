/*
 * Auto Install File 1.0 By Gamer_Z
 */

#define PRV (1)
//-------------------------------------------//
#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <iostream> 
#include <fstream>
#include <sstream> 
#include <vector>
#include <string>
#include <utility>
#include <algorithm>
#include <iterator>
#include <cstdlib>
#include <stdexcept>

//
/*
	Credits go to LucDalton from stackoverflow for providing the server.cfg parser/editor.
*/
//using line = std::string;
typedef std::string line;
//using vector_type = std::vector<line>;
typedef std::vector<line> vector_type;
//using line_iterator = vector_type::const_iterator;
typedef vector_type::const_iterator line_iterator;

vector_type to_lines(std::istream& is)
{
    vector_type result;

    line l;
    while(std::getline(is, l)) {
        result.push_back(std::move(l));
    }

    return result;
}

//using key_type = std::string;
typedef std::string key_type;
//using value_type = std::vector<std::string>;
typedef std::vector<std::string> value_type;
//using pair_type = std::pair<key_type, value_type>;
typedef std::pair<key_type, value_type> pair_type;
//using map_type = std::vector<pair_type>;
typedef std::vector<pair_type> map_type;

map_type to_pairs(vector_type const& lines)
{
    map_type result;

    //for(auto&& line: lines) {
	
	for(std::vector<line>::const_iterator it = lines.begin(); it != lines.end(); ++it)
	{
		line const& line = *it;
        std::istringstream stream(line);
        std::string key;
        if(stream >> key) {
            value_type value;
            std::copy(std::istream_iterator<std::string> ( stream ), std::istream_iterator<std::string> (), std::back_inserter(value));
            result.push_back(std::make_pair(std::move(key), std::move(value)));
        }
    };

    return result;
}

int update_key(map_type& map, key_type const& key, value_type const& value)
{
    map_type::iterator it;
    auto predicate = [&key](pair_type const& pair) { return pair.first == key; };
    if((it = std::find_if(map.begin(), map.end(), predicate)) != map.end()) {
        std::copy(value.begin(), value.end(), std::back_inserter(it->second));
    } else {
        //std::cout << "Uh oh! no key to update\n" ;
		return 0;
    }
	return 1;
}

vector_type
from_pairs(map_type const& map)
{
    vector_type result;

    //for(auto const& pair: map) 
	for(std::vector< pair_type>::const_iterator it = map.begin(); it != map.end(); ++it)
	{
		pair_type const& pair = *it;
        std::string line = pair.first;
        //for(auto&& value: pair.second) 
		for(std::vector<std::string>::const_iterator it = pair.second.begin(); it != pair.second.end(); ++it)
		{
			std::string const& value = *it;
            line += " " + value;
        }
        result.push_back(std::move(line));
    }

    return result;
}

void
from_lines(std::ostream& os, vector_type const& lines)
{
    std::copy(lines.begin(), lines.end(), std::ostream_iterator<std::string>(  os, "\n" ));
}
//-------------------------------------------//
#include "./SDK/amx/amx.h"
#include "./SDK/plugincommon.h"
#define SI_SUPPORT_IOSTREAMS
#include "./INI/SimpleIni.h"
#include "./ZIP/win/unzip.h"
using namespace std;

typedef void (*logprintf_t)(char* format, ...);
logprintf_t logprintf;
extern void *pAMXFunctions;
//-------------------------------------------//
#ifdef WIN32
#include <Windows.h>

/***************************/
/* ansi-unicode conversion */
/***************************/

BOOL AnsiToUnicode16(CHAR *in_Src, WCHAR *out_Dst, INT in_MaxLen)
{
    /* locals */
    INT lv_Len;

  // do NOT decrease maxlen for the eos
  if (in_MaxLen <= 0)
    return FALSE;

  // let windows find out the meaning of ansi
  // - the SrcLen=-1 triggers MBTWC to add a eos to Dst and fails if MaxLen is too small.
  // - if SrcLen is specified then no eos is added
  // - if (SrcLen+1) is specified then the eos IS added
  lv_Len = MultiByteToWideChar(CP_ACP, 0, in_Src, -1, out_Dst, in_MaxLen);

  // validate
  if (lv_Len < 0)
    lv_Len = 0;

  // ensure eos, watch out for a full buffersize
  // - if the buffer is full without an eos then clear the output like MBTWC does
  //   in case of too small outputbuffer
  // - unfortunately there is no way to let MBTWC return shortened strings,
  //   if the outputbuffer is too small then it fails completely
  if (lv_Len < in_MaxLen)
    out_Dst[lv_Len] = 0;
  else if (out_Dst[in_MaxLen-1])
    out_Dst[0] = 0;

  // done
  return TRUE;
}


BOOL AnsiToUnicode16L(CHAR *in_Src, INT in_SrcLen, WCHAR *out_Dst, INT in_MaxLen)
{
    /* locals */
    INT lv_Len;


  // do NOT decrease maxlen for the eos
  if (in_MaxLen <= 0)
    return FALSE;

  // let windows find out the meaning of ansi
  // - the SrcLen=-1 triggers MBTWC to add a eos to Dst and fails if MaxLen is too small.
  // - if SrcLen is specified then no eos is added
  // - if (SrcLen+1) is specified then the eos IS added
  lv_Len = MultiByteToWideChar(CP_ACP, 0, in_Src, in_SrcLen, out_Dst, in_MaxLen);

  // validate
  if (lv_Len < 0)
    lv_Len = 0;

  // ensure eos, watch out for a full buffersize
  // - if the buffer is full without an eos then clear the output like MBTWC does
  //   in case of too small outputbuffer
  // - unfortunately there is no way to let MBTWC return shortened strings,
  //   if the outputbuffer is too small then it fails completely
  if (lv_Len < in_MaxLen)
    out_Dst[lv_Len] = 0;
  else if (out_Dst[in_MaxLen-1])
    out_Dst[0] = 0;

  // done
  return TRUE;
}

/***************************/
/* unicode-ansi conversion */
/***************************/
BOOL Unicode16ToAnsi(WCHAR *in_Src, CHAR *out_Dst, INT in_MaxLen)
{
    /* locals */
    INT  lv_Len;
    BOOL lv_UsedDefault;
  
  // do NOT decrease maxlen for the eos
  if (in_MaxLen <= 0)
    return FALSE;

  // let windows find out the meaning of ansi
  // - the SrcLen=-1 triggers WCTMB to add a eos to Dst and fails if MaxLen is too small.
  // - if SrcLen is specified then no eos is added
  // - if (SrcLen+1) is specified then the eos IS added
  lv_Len = WideCharToMultiByte(
     CP_ACP, 0, in_Src, -1, out_Dst, in_MaxLen, 0, &lv_UsedDefault);

  // validate
  if (lv_Len < 0)
    lv_Len = 0;

  // ensure eos, watch out for a full buffersize
  // - if the buffer is full without an eos then clear the output like WCTMB does
  //   in case of too small outputbuffer
  // - unfortunately there is no way to let WCTMB return shortened strings,
  //   if the outputbuffer is too small then it fails completely
  if (lv_Len < in_MaxLen)
    out_Dst[lv_Len] = 0;
  else if (out_Dst[in_MaxLen-1])
    out_Dst[0] = 0;

  // return whether invalid chars were present
  return !lv_UsedDefault;
}
BOOL Unicode16ToAnsiL(WCHAR *in_Src, INT in_SrcLen, CHAR *out_Dst, INT in_MaxLen)
{
    /* locals */
    INT  lv_Len;
    BOOL lv_UsedDefault;
  
  // do NOT decrease maxlen for the eos
  if (in_MaxLen <= 0)
    return FALSE;

  // let windows find out the meaning of ansi
  // - the SrcLen=-1 triggers WCTMB to add a eos to Dst and fails if MaxLen is too small.
  // - if SrcLen is specified then no eos is added
  // - if (SrcLen+1) is specified then the eos IS added
  lv_Len = WideCharToMultiByte(
     CP_ACP, 0, in_Src, in_SrcLen, out_Dst, in_MaxLen, 0, &lv_UsedDefault);

  // validate
  if (lv_Len < 0)
    lv_Len = 0;

  // ensure eos, watch out for a full buffersize
  // - if the buffer is full without an eos then clear the output like WCTMB does
  //   in case of too small outputbuffer
  // - unfortunately there is no way to let WCTMB return shortened strings,
  //   if the outputbuffer is too small then it fails completely
  if (lv_Len < in_MaxLen)
    out_Dst[lv_Len] = 0;
  else if (out_Dst[in_MaxLen-1])
    out_Dst[0] = 0;

  // return whether invalid chars were present
  return !lv_UsedDefault;
}

#else//assuming linux

#endif

int SetVersion(char * key,int value){
	CSimpleIniA ini(true,false,false);

	SI_Error rc = ini.LoadFile("PACKAGES.AIFLIST");
	if (rc < 0){
		ini.SaveFile("PACKAGES.AIFLIST");
	}
	rc = ini.LoadFile("PACKAGES.AIFLIST");
	if (rc < 0){
		return 0;
	}
	char valr[128];
	_itoa (value,valr,10);
	ini.SetValue("AUTO_INSTALLER_FILE_1_0",key,valr,NULL,true);
	ini.SaveFile("PACKAGES.AIFLIST");
	return 1;
}

int GetVersion(char * key){
	//logprintf("2");
	CSimpleIniA ini(true,false,false);
	//logprintf("3");
	SI_Error rc = ini.LoadFile("PACKAGES.AIFLIST");
	//logprintf("4");
	if (rc < 0){
		return 0;
	}
	//logprintf("5");
	const char * pVal = ini.GetValue("AUTO_INSTALLER_FILE_1_0",key,NULL,NULL);
	//logprintf("6 %s",pVal);
	ini.Reset();
	if(pVal == NULL)return 0;
	//logprintf("2");
	return atoi(pVal);
}

string CharToStr(char * chars)
{
	stringstream ss;
	string s;
	ss << chars;
	ss >> s;
	return s;
}

char _PrivSTR_to_up[256];
char * toUpper(char * str) {
    for(unsigned int x=0; x < strlen(str); x++)
        _PrivSTR_to_up[x]=toupper(str[x]);
	return _PrivSTR_to_up;
}

void AddToServerCfg(char * key,char * data)
{
	std::ifstream file("server.cfg");
	if(!file) {
		cout << "Bad input file!\n";
	}
	else
	{

		auto lines = to_lines(file);
		file.close();
		auto pairs = to_pairs(lines);
				
		value_type x;
		x.push_back(data);
		bool DoUpdate = true;
		for(std::vector<std::string>::const_iterator it = lines.begin(); it != lines.end(); ++it)
		{
			std::string value = *it;
			transform(value.begin(),value.end(),value.begin(),toupper);
			if(value.find(toUpper(data) /*char */) != string::npos)
			{
				DoUpdate = false;
				break;
			}
		}
		if(DoUpdate == true)
		{
			if(update_key(pairs, key, x) == 0)//Uh oh not found
			{
				lines.push_back(key);
				pairs = to_pairs(lines);
				update_key(pairs, key, x);
			}
		}
		std::ofstream out("~server.cfg");
		if(!out) {
			cout << "Bad output file!\n";
		}
		else
		{
			from_lines(out, from_pairs(pairs));
			out.clear();
			out.seekp(0);
			out.close();
			if(remove("server.cfg") == 0)
			{

			}
			else
			{
				logprintf("Error removing old server.cfg");
			}
			if(rename("~server.cfg","server.cfg") == 0)
			{

			}
			else
			{
				remove("~server.cfg");
				logprintf("Error renaming to new server.cfg");
			}
		}
	}
}

PLUGIN_EXPORT unsigned int PLUGIN_CALL Supports() 
{
	return SUPPORTS_VERSION;
}

struct ToDoInfo
{
	int action;//0 - unpack, 1 - add plugin to server.cfg line if not exists, 2 add fs..., 3 print msg
	char string1[256];
	char string2[256];
	ToDoInfo(int _action,char* _string1,char* _string2)
	{
		action = _action;
		sprintf(string1,"%s", _string1);
		sprintf(string2,"%s", _string2);
	}
};

vector<ToDoInfo> ToDoVec;

struct PAKinfo
{
	char NAME[256];
	char ID[256];
	char AUTHOR[256];
	int VERSION;
	PAKinfo(char * _NAME,char * _ID,char * _AUTHOR, int _VERSION)
	{
		sprintf(NAME,"%s", _NAME);
		sprintf(ID,"%s", _ID);
		sprintf(AUTHOR,"%s", _AUTHOR);
		VERSION	= _VERSION;
	}
};

vector<PAKinfo> PAKinfovec;

PLUGIN_EXPORT bool PLUGIN_CALL Load( void **ppData ) 
{
    pAMXFunctions = ppData[PLUGIN_DATA_AMX_EXPORTS];
	logprintf = (logprintf_t) ppData[PLUGIN_DATA_LOGPRINTF];
	logprintf("\t\t AIF R%d -- Looking for packages...\r\n",PRV);
	#ifdef WIN32

	char buf[65535];
	bool found = false;
	bool dobreak = false;
	bool stopupdate = false;
	bool install = false;
	bool installing = false;
	unsigned int installed = 0;


	WIN32_FIND_DATA fd;
	HANDLE h = FindFirstFile(L"./*.AIFPAK", &fd);

	char NAME[256] = "";
	char REQUIRE[64] = "";
	char ID[256] = "";
	char AUTHOR[256] = "";
	int VERSION;

	HZIP hz = OpenZip(fd.cFileName,0);

	ZIPENTRY ze; 
	GetZipItem(hz,-1,&ze); 
	int numitems=ze.index;
	for (int i=0; i<numitems; ++i)
	{ 
		GetZipItem(hz,i,&ze);
		
		if(found == false)
		{
			if(_wcsicmp(ze.name,L"INSTALL.AIFLIST") == 0)
			{
				found = true;
				char packname[128];
				Unicode16ToAnsi(fd.cFileName,packname,128);
				logprintf("Found valid package: %s",packname);
				UnzipItem(hz,i,buf,65535);
				i = 0;
				stringstream strx;
				strx << buf;
				string lineoo;     
				while (getline(strx, lineoo)) { 
					char * first = NULL;
					char * second = NULL;
					char * third = NULL;
					char * linex = NULL;
					if(lineoo.length() < 3)continue;
					linex = (char *)lineoo.c_str();
					first = strtok (linex," \r\n");
					if(install == false)
					{
						if(_stricmp(first,"REQUIRE_PACKAGE") == 0)
						{
							sprintf(REQUIRE,"%s",strtok (NULL," "));
							int MinVer = atoi(strtok (NULL," "));
							if(GetVersion(REQUIRE) < MinVer)
							{
								install = true;
								dobreak = true;
								stopupdate = true;
								logprintf("Package '%s' (Minimal Version %d) is required to install this package. Please install the needed package first. Skipping installation...",strtok (NULL,"\r\n"),MinVer);
							}
						}
						if(_stricmp(first,"NAME") == 0)
						{
							sprintf(NAME,"%s",strtok (NULL,"\r\n"));
						}
						if(_stricmp(first,"ID") == 0)
						{
							sprintf(ID,"%s",strtok (NULL,"\r\n"));
						}
						if(_stricmp(first,"AUTHOR") == 0)
						{
							sprintf(AUTHOR,"%s",strtok (NULL,"\r\n"));
						}
						if(_stricmp(first,"VERSION") == 0)
						{
							VERSION = atoi(strtok (NULL,"\r\n"));
						}
						if(_stricmp(first,"START_INSTALL") == 0)
						{
							second = strtok (NULL,"\r\n");
							if(_stricmp(second,"WINDOWS") == 0)
							{
								install = true;
								PAKinfovec.push_back(PAKinfo(NAME,ID,AUTHOR,VERSION));
							}
						}
					}
					else if(dobreak == false)
					{
						if(_stricmp(first,"UNPACK") == 0)
						{
							second = strtok (NULL,">");
							third = strtok (NULL,"\r\n");
							ToDoVec.push_back(ToDoInfo(0,second,third));
						}
						if(_stricmp(first,"PRINT") == 0)
						{
							second = strtok (NULL,"\r\n");
							ToDoVec.push_back(ToDoInfo(3,second,""));
						}
						if(_stricmp(first,"ADD_PLUGIN") == 0)
						{
							second = strtok (NULL,"\r\n");
							ToDoVec.push_back(ToDoInfo(1,second,""));
						}
						if(_stricmp(first,"ADD_FILTERSCRIPT") == 0)
						{
							second = strtok (NULL,"\r\n");
							ToDoVec.push_back(ToDoInfo(2,second,""));
						}
						if(_stricmp(first,"END_INSTALL") == 0)
						{
							dobreak = true;
						}
					}
				} 
			}
		}
		else if(stopupdate == false)
		{
			if(installing == false)
			{
				i = -1;
				if(PAKinfovec.at(0).VERSION < GetVersion(PAKinfovec.at(0).ID))
				{
					logprintf("It seems that the current installed package is newer than the AIFPAK");
					stopupdate = true;
				}
				else
				if(PAKinfovec.at(0).VERSION == GetVersion(PAKinfovec.at(0).ID))
				{
					logprintf("Package is up-to-date");
					stopupdate = true;
				}
				else
				{
					logprintf("Updating server package '%s' By '%s' from version '%d' to '%d' .",PAKinfovec.at(0).NAME,PAKinfovec.at(0).AUTHOR,GetVersion(PAKinfovec.at(0).ID),PAKinfovec.at(0).VERSION);
					SetVersion(PAKinfovec.at(0).ID,PAKinfovec.at(0).VERSION);
					++installed;
				}
				installing = true;
			}
			else
			{
				for(unsigned int ix = 0; ix < ToDoVec.size(); ++ix)
				{
					if(ToDoVec.at(ix).action == 0)
					{
						TCHAR tempstr[260] = L"";
						TCHAR unpackloc[260] = L"";
						AnsiToUnicode16(ToDoVec.at(ix).string1,tempstr,260);
						AnsiToUnicode16(ToDoVec.at(ix).string2,unpackloc,260);
						if(_wcsicmp(ze.name,tempstr) == 0)
						{
							UnzipItem(hz,i,unpackloc);
							break;
						}
					}
				}
			}
		}
	}
	while (!ToDoVec.empty())
	{
		if(stopupdate == false)
		{
			if(ToDoVec.back().action == 3)
			{
				logprintf("%s",ToDoVec.back().string1);
			}
			if(ToDoVec.back().action == 2)//add a filterscript to the 'filterscript' line if not added already, create the line if it doesn't exists
			{
				AddToServerCfg("filterscripts",ToDoVec.back().string1);
			}
			if(ToDoVec.back().action == 1)//add a plugin to the 'plugins' line if not added already, create the line if it doesn't exists
			{
				AddToServerCfg("plugins",ToDoVec.back().string1);
			}
		}
		ToDoVec.pop_back();
	}
	while (!PAKinfovec.empty())
	{
		PAKinfovec.pop_back();
	}
	found = false;
	install = false;
	installing = false;
	dobreak = false;
	stopupdate = false;
	CloseZip(hz);

	while (FindNextFile(h, &fd))
	{
		hz = OpenZip(fd.cFileName,0);
		GetZipItem(hz,-1,&ze); 
		int numitems=ze.index;
		for (int i=0; i<numitems; ++i)
		{ 
			GetZipItem(hz,i,&ze);
		
			if(found == false)
			{
				if(_wcsicmp(ze.name,L"INSTALL.AIFLIST") == 0)
				{
					found = true;
					char packname[128];
					Unicode16ToAnsi(fd.cFileName,packname,128);
					logprintf("Found valid package: %s",packname);
					UnzipItem(hz,i,buf,65535);
					i = 0;
					stringstream strx;
					strx << buf;
					string lineoo;     
					while (getline(strx, lineoo)) { 
						char * first = NULL;
						char * second = NULL;
						char * third = NULL;
						char * linex = NULL;
						if(lineoo.length() < 3)continue;
						linex = (char *)lineoo.c_str();
						first = strtok (linex," \r\n");
						if(install == false)
						{
							if(_stricmp(first,"REQUIRE_PACKAGE") == 0)
							{
								sprintf(REQUIRE,"%s",strtok (NULL," "));
								int MinVer = atoi(strtok (NULL," "));
								if(GetVersion(REQUIRE) < MinVer)
								{
									install = true;
									dobreak = true;
									stopupdate = true;
									logprintf("Package '%s' (Version %d) is required to install this package. Skipping installation...",strtok (NULL,"\r\n"),MinVer);
								}
							}
							if(_stricmp(first,"NAME") == 0)
							{
								sprintf(NAME,"%s",strtok (NULL,"\r\n"));
							}
							if(_stricmp(first,"ID") == 0)
							{
								sprintf(ID,"%s",strtok (NULL,"\r\n"));
							}
							if(_stricmp(first,"AUTHOR") == 0)
							{
								sprintf(AUTHOR,"%s",strtok (NULL,"\r\n"));
							}
							if(_stricmp(first,"VERSION") == 0)
							{
								VERSION = atoi(strtok (NULL,"\r\n"));
							}
							if(_stricmp(first,"START_INSTALL") == 0)
							{
								second = strtok (NULL,"\r\n");
								if(_stricmp(second,"WINDOWS") == 0)
								{
									install = true;
									PAKinfovec.push_back(PAKinfo(NAME,ID,AUTHOR,VERSION));
								}
							}
						}
						else if(dobreak == false)
						{
							if(_stricmp(first,"UNPACK") == 0)
							{
								second = strtok (NULL,">");
								third = strtok (NULL,"\r\n");
								ToDoVec.push_back(ToDoInfo(0,second,third));
							}
							if(_stricmp(first,"PRINT") == 0)
							{
								second = strtok (NULL,"\r\n");
								ToDoVec.push_back(ToDoInfo(3,second,""));
							}
							if(_stricmp(first,"ADD_PLUGIN") == 0)
							{
								second = strtok (NULL,"\r\n");
								ToDoVec.push_back(ToDoInfo(1,second,""));
							}
							if(_stricmp(first,"ADD_FILTERSCRIPT") == 0)
							{
								second = strtok (NULL,"\r\n");
								ToDoVec.push_back(ToDoInfo(2,second,""));
							}
							if(_stricmp(first,"END_INSTALL") == 0)
							{
								dobreak = true;
							}
						}
					} 
				}
			}
			else if(stopupdate == false)
			{
				if(installing == false)
				{
					i = -1;
					if(PAKinfovec.at(0).VERSION < GetVersion(PAKinfovec.at(0).ID))
					{
						logprintf("It seems that the current installed package is newer than the AIFPAK");
						stopupdate = true;
					}
					else
					if(PAKinfovec.at(0).VERSION == GetVersion(PAKinfovec.at(0).ID))
					{
						logprintf("Package is up-to-date");
						stopupdate = true;
					}
					else
					{
						logprintf("Updating server package '%s' By '%s' from version '%d' to '%d' .",PAKinfovec.at(0).NAME,PAKinfovec.at(0).AUTHOR,GetVersion(PAKinfovec.at(0).ID),PAKinfovec.at(0).VERSION);
						SetVersion(PAKinfovec.at(0).ID,PAKinfovec.at(0).VERSION);
						++installed;
					}
					installing = true;
				}
				else
				{
					for(unsigned int ix = 0; ix < ToDoVec.size(); ++ix)
					{
						if(ToDoVec.at(ix).action == 0)
						{
							TCHAR tempstr[260] = L"";
							TCHAR unpackloc[260] = L"";
							AnsiToUnicode16(ToDoVec.at(ix).string1,tempstr,260);
							AnsiToUnicode16(ToDoVec.at(ix).string2,unpackloc,260);
							if(_wcsicmp(ze.name,tempstr) == 0)
							{
								UnzipItem(hz,i,unpackloc);
								break;
							}
						}
					}
				}
			}
		}
		while (!ToDoVec.empty())
		{
			if(stopupdate == false)
			{
				if(ToDoVec.back().action == 3)
				{
					logprintf("%s",ToDoVec.back().string1);
				}
				if(ToDoVec.back().action == 2)//add a filterscript to the 'filterscript' line if not added already, create the line if it doesn't exists
				{
					AddToServerCfg("filterscripts",ToDoVec.back().string1);
				}
				if(ToDoVec.back().action == 1)//add a plugin to the 'plugins' line if not added already, create the line if it doesn't exists
				{
					AddToServerCfg("plugins",ToDoVec.back().string1);
				}
			}
			ToDoVec.pop_back();
		}
		while (!PAKinfovec.empty())
		{
			PAKinfovec.pop_back();
		}
		found = false;
		install = false;
		installing = false;
		dobreak = false;
		stopupdate = false;
		CloseZip(hz);
	}

	if(installed > 0)
	{
		logprintf("Please restart your server to apply package updates/installations.");
	}
	else
	{
		logprintf("All detected packages are up-to-date.");
	}
	#else//assuming linux

	#endif
	logprintf("\r\n");
	return true;
}
