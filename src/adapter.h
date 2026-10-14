// Chain TESMIO's current import handlers; never replace original/VFS data files.
#pragma once
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <mutex>
#include <functional>
#include <cmath>
#include <cstdlib>
#include <algorithm>
#include "tesmio_api.h"
namespace Adapter {
namespace fs=std::filesystem;
inline const TsmHost* H=nullptr;
inline std::string name,ini;
inline fs::path game,plugins,cache;
inline bool enabled=false,vfs=false,ready=false,startAttempted=false;
inline std::function<bool(const std::string&)> accepts;
inline std::function<std::string(const std::string&,const std::string&)> transform;
inline std::function<fs::path(const std::string&)> asset;
inline std::mutex lock;
inline unsigned serial=0;
inline std::map<std::wstring,std::pair<std::string,fs::path>> memo;
inline std::wstring Wide(const std::string& s) {
 if(s.empty())return {};
 int n=MultiByteToWideChar(CP_UTF8,MB_ERR_INVALID_CHARS,s.c_str(),-1,nullptr,0);
 UINT cp=CP_UTF8;if(!n){cp=CP_ACP;n=MultiByteToWideChar(cp,0,s.c_str(),-1,nullptr,0);}
 if(!n)return {}; std::wstring r(n,0);MultiByteToWideChar(cp,0,s.c_str(),-1,r.data(),n);r.pop_back();return r;
}
inline std::string Utf8(const std::wstring& s){
 if(s.empty())return {};int n=WideCharToMultiByte(CP_UTF8,0,s.c_str(),-1,nullptr,0,nullptr,nullptr);
 std::string r(n,0);WideCharToMultiByte(CP_UTF8,0,s.c_str(),-1,r.data(),n,nullptr,nullptr);r.pop_back();return r;
}
inline std::string Normalize(std::string s){
 for(char& c:s){if(c=='\\')c='/';else if(c>='A'&&c<='Z')c=static_cast<char>(c+32);}return s;
}
inline std::vector<std::string> Lines(const std::string& s){
 std::vector<std::string> r;std::istringstream f(s);std::string l;while(std::getline(f,l)){if(!l.empty()&&l.back()=='\r')l.pop_back();r.push_back(l);}return r;
}
inline std::string Join(const std::vector<std::string>& v){std::string r;for(auto& l:v)r+=l+"\r\n";return r;}
inline std::string Num(double v){char b[64];sprintf_s(b,"%.9g",v);return b;}
inline double Setting(const std::string& section,const std::string& key,double fallback,double low=0.05,double high=20){
 char b[96]={};auto def=Num(fallback);H->configString(ini.c_str(),section.c_str(),key.c_str(),b,sizeof(b),def.c_str());
 char* end=nullptr;double n=strtod(b,&end);while(end&&isspace(static_cast<unsigned char>(*end)))++end;
 if(!end||end==b||*end||!std::isfinite(n)||n<low||n>high)throw std::runtime_error(section+"."+key+" outside ["+Num(low)+","+Num(high)+"]");return n;
}
inline bool Toggle(const char* section,const char* key,int def){int n=H->configInt(ini.c_str(),section,key,def);if(n!=0&&n!=1)throw std::runtime_error(std::string(key)+" must be 0 or 1");return n==1;}
inline void Setup(const TsmHost* h,const char* mod){
 H=h;name=mod;ini="plugins\\"+name+".ini";
 wchar_t exe[32768]={};GetModuleFileNameW(static_cast<HMODULE>(H->exeModule),exe,32768);game=fs::path(exe).parent_path();
 plugins=fs::path(Wide(H->pluginDir?H->pluginDir:""));
 vfs=H->configInt("tesmioloader.ini","tesmioloader","vfs",1)!=0;
 wchar_t tmp[32768]={};DWORD n=GetTempPathW(32768,tmp);if(!n||n>=32768)throw std::runtime_error("temp path unavailable");
 cache=fs::path(tmp)/L"TESMIO"/Wide(name)/(L"process_"+std::to_wstring(GetCurrentProcessId()));
}
inline bool Read(const fs::path& p,std::string& s){std::ifstream f(p,std::ios::binary);if(!f)return false;f.seekg(0,std::ios::end);auto n=f.tellg();if(n<0||n>4*1024*1024)return false;f.seekg(0);s.assign(static_cast<size_t>(n),'\0');return !n||static_cast<bool>(f.read(s.data(),n));}
inline fs::path Redirect(const wchar_t* input){
 if(!enabled||!ready||!input)return {};
 try {
  std::string key=Normalize(Utf8(input));
  if(asset){auto p=asset(key);if(!p.empty()&&fs::is_regular_file(p))return p;}
  if(!accepts||!accepts(key))return {};
  fs::path path(input),source=path;
  if(path.is_relative()){
   if(vfs&&H->vfsRoot){auto vp=fs::path(Wide(H->vfsRoot))/path;if(fs::is_regular_file(vp))source=vp;else source=game/path;}
   else source=game/path;
  }
  std::lock_guard<std::mutex> guard(lock);
  std::string bytes;if(!Read(source,bytes))return {};
  auto found=memo.find(source.wstring());if(found!=memo.end()&&found->second.first==bytes&&fs::is_regular_file(found->second.second))return found->second.second;
  std::string changed=transform(key,bytes);if(changed==bytes)return {};
  fs::create_directories(cache);auto output=cache/(L"content_"+std::to_wstring(++serial)+L".dat");
  {std::ofstream f(output,std::ios::binary|std::ios::trunc);f.write(changed.data(),static_cast<std::streamsize>(changed.size()));f.close();if(!f)throw std::runtime_error("cache write failed");}
  memo[source.wstring()]={bytes,output};H->log("%s transformed %s -> temporary data (%zu bytes)",name.c_str(),Utf8(source.wstring()).c_str(),changed.size());return output;
 }catch(const std::exception& e){H->log("%s file transform skipped: %s",name.c_str(),e.what());return {};}
}
using Fopen=FILE*(*)(const char*,const char*);using FopenS=errno_t(*)(FILE**,const char*,const char*);
using Wfopen=FILE*(*)(const wchar_t*,const wchar_t*);using WfopenS=errno_t(*)(FILE**,const wchar_t*,const wchar_t*);
using ReadBuffer=int(*)(const char*,char**,unsigned*,bool);using Exists=bool(*)(const char*,bool,bool);
struct Imports {Fopen fopen=nullptr;FopenS fopen_s=nullptr;Wfopen wfopen=nullptr;WfopenS wfopen_s=nullptr;ReadBuffer read=nullptr;Exists exists=nullptr;
 decltype(&CreateFileA) createA=nullptr;decltype(&CreateFileW) createW=nullptr;decltype(&CreateFile2) create2=nullptr;};
inline Imports originals[2];
inline bool Reading(const char* m){return m&&m[0]=='r'&&!strchr(m,'+');}
inline bool Reading(const wchar_t* m){return m&&m[0]==L'r'&&!wcschr(m,L'+');}
template<int I> FILE* Open(const char* p,const char* m){auto q=Reading(m)?Redirect(Wide(p?p:"").c_str()):fs::path();auto a=q.empty()?std::string():Utf8(q.wstring());return originals[I].fopen(q.empty()?p:a.c_str(),m);}
template<int I> errno_t OpenS(FILE** f,const char* p,const char* m){auto q=Reading(m)?Redirect(Wide(p?p:"").c_str()):fs::path();auto a=q.empty()?std::string():Utf8(q.wstring());return originals[I].fopen_s(f,q.empty()?p:a.c_str(),m);}
template<int I> FILE* WOpen(const wchar_t* p,const wchar_t* m){auto q=Reading(m)?Redirect(p):fs::path();return originals[I].wfopen(q.empty()?p:q.c_str(),m);}
template<int I> errno_t WOpenS(FILE** f,const wchar_t* p,const wchar_t* m){auto q=Reading(m)?Redirect(p):fs::path();return originals[I].wfopen_s(f,q.empty()?p:q.c_str(),m);}
template<int I> int Buffer(const char* p,char** b,unsigned* n,bool f){auto q=Redirect(Wide(p?p:"").c_str());auto a=q.empty()?std::string():Utf8(q.wstring());return originals[I].read(q.empty()?p:a.c_str(),b,n,f);}
template<int I> bool Exist(const char* p,bool a,bool b){auto q=asset?asset(Normalize(p?p:"")):fs::path();if(!q.empty()&&fs::is_regular_file(q))return true;return originals[I].exists(p,a,b);}
template<int I> HANDLE WINAPI CreateA(LPCSTR p,DWORD access,DWORD share,LPSECURITY_ATTRIBUTES sec,DWORD disp,DWORD flags,HANDLE t){auto q=(disp==OPEN_EXISTING&&!(access&(GENERIC_WRITE|DELETE|FILE_WRITE_DATA|FILE_APPEND_DATA)))?Redirect(Wide(p?p:"").c_str()):fs::path();auto a=q.empty()?std::string():Utf8(q.wstring());return originals[I].createA(q.empty()?p:a.c_str(),access,share,sec,disp,flags,t);}
template<int I> HANDLE WINAPI CreateW(LPCWSTR p,DWORD access,DWORD share,LPSECURITY_ATTRIBUTES sec,DWORD disp,DWORD flags,HANDLE t){auto q=(disp==OPEN_EXISTING&&!(access&(GENERIC_WRITE|DELETE|FILE_WRITE_DATA|FILE_APPEND_DATA)))?Redirect(p):fs::path();return originals[I].createW(q.empty()?p:q.c_str(),access,share,sec,disp,flags,t);}
template<int I> HANDLE WINAPI Create2(LPCWSTR p,DWORD access,DWORD share,DWORD disp,LPCREATEFILE2_EXTENDED_PARAMETERS options){auto q=(disp==OPEN_EXISTING&&!(access&(GENERIC_WRITE|DELETE|FILE_WRITE_DATA|FILE_APPEND_DATA)))?Redirect(p):fs::path();return originals[I].create2(q.empty()?p:q.c_str(),access,share,disp,options);}
inline bool InstallSlot(void* module,const char* dll,const char* function,void* hook,void** original){
 if(!module||!H->findIatSlot(module,dll,function))return false;
 auto label=name+" "+function;return H->patchIat(module,dll,function,hook,original,label.c_str())!=0;
}
template<int I> unsigned Install(void* module){
 unsigned n=0;auto& o=originals[I];const char* crt="api-ms-win-crt-stdio-l1-1-0.dll";const char* engine="C3DDLL64.dll";
 n+=InstallSlot(module,crt,"fopen",reinterpret_cast<void*>(&Open<I>),reinterpret_cast<void**>(&o.fopen));
 n+=InstallSlot(module,crt,"fopen_s",reinterpret_cast<void*>(&OpenS<I>),reinterpret_cast<void**>(&o.fopen_s));
 n+=InstallSlot(module,crt,"_wfopen",reinterpret_cast<void*>(&WOpen<I>),reinterpret_cast<void**>(&o.wfopen));
 n+=InstallSlot(module,crt,"_wfopen_s",reinterpret_cast<void*>(&WOpenS<I>),reinterpret_cast<void**>(&o.wfopen_s));
 n+=InstallSlot(module,engine,"?C3DHelp_ReadFileIntoBuffer@@YAHPEBDPEAPEADPEAI_N@Z",reinterpret_cast<void*>(&Buffer<I>),reinterpret_cast<void**>(&o.read));
 n+=InstallSlot(module,engine,"?C3DHelp_CheckIfFileExist@@YA_NPEBD_N1@Z",reinterpret_cast<void*>(&Exist<I>),reinterpret_cast<void**>(&o.exists));
 for(auto dll:{"KERNEL32.dll","KERNELBASE.dll","api-ms-win-core-file-l1-1-0.dll","api-ms-win-core-file-l1-2-0.dll"}){
  if(!o.createA)n+=InstallSlot(module,dll,"CreateFileA",reinterpret_cast<void*>(&CreateA<I>),reinterpret_cast<void**>(&o.createA));
  if(!o.createW)n+=InstallSlot(module,dll,"CreateFileW",reinterpret_cast<void*>(&CreateW<I>),reinterpret_cast<void**>(&o.createW));
  if(!o.create2)n+=InstallSlot(module,dll,"CreateFile2",reinterpret_cast<void*>(&Create2<I>),reinterpret_cast<void**>(&o.create2));
 }return n;
}
inline int Start(){if(!enabled||startAttempted)return 0;startAttempted=true;ready=true;unsigned n=Install<0>(H->exeModule)+Install<1>(H->engineModule);
 H->log("%s chained %u file imports; original/VFS data retained; cache=%s",name.c_str(),n,Utf8(cache.wstring()).c_str());
 if(!originals[0].fopen){ready=false;H->log("%s inactive: required EXE fopen hook unavailable",name.c_str());}return 0;
}
inline bool ValidHost(const TsmHost* h){return h&&h->apiVersion>=4&&h->structSize>=sizeof(TsmHost)&&h->log&&h->configInt&&h->configString&&h->findIatSlot&&h->patchIat&&h->exeModule&&h->pluginDir;}
}
