#pragma once
#include <bcrypt.h>
#include <cstdint>
#include <cstring>
namespace NativeResearch {
bool ExactExe(const TsmHost* H) {
    wchar_t path[32768] = {};
    if (!GetModuleFileNameW(static_cast<HMODULE>(H->exeModule),path,32768)) return false;
    HANDLE file = CreateFileW(path,GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,
                             nullptr,OPEN_EXISTING,FILE_FLAG_SEQUENTIAL_SCAN,nullptr);
    if (file == INVALID_HANDLE_VALUE) return false;
    BCRYPT_ALG_HANDLE alg = nullptr; BCRYPT_HASH_HANDLE hash = nullptr;
    DWORD objectSize=0,got=0; unsigned char digest[32] = {};
    bool ok = BCryptOpenAlgorithmProvider(&alg,BCRYPT_SHA256_ALGORITHM,nullptr,0)>=0;
    if (ok) ok = BCryptGetProperty(alg,BCRYPT_OBJECT_LENGTH,reinterpret_cast<PUCHAR>(&objectSize),
                                  sizeof(objectSize),&got,0)>=0;
    std::vector<unsigned char> object(objectSize), buffer(65536);
    if (ok) ok = BCryptCreateHash(alg,&hash,object.data(),objectSize,nullptr,0,0)>=0;
    while (ok) {
        DWORD n=0;
        if (!ReadFile(file,buffer.data(),static_cast<DWORD>(buffer.size()),&n,nullptr)) {ok=false;break;}
        if (!n) break;
        ok = BCryptHashData(hash,buffer.data(),n,0)>=0;
    }
    if (ok) ok = BCryptFinishHash(hash,digest,sizeof(digest),0)>=0;
    if (hash) BCryptDestroyHash(hash);
    if (alg) BCryptCloseAlgorithmProvider(alg,0);
    CloseHandle(file);
    char hex[65]={};
    for (unsigned i=0;i<32;++i) std::sprintf(hex+2*i,"%02x",digest[i]);
    return ok && std::strcmp(hex,"296644a9f207d609031fc2ae73fed2dcb34619a1d55a35d1c7b51965ce6841b8")==0;
}

inline bool Write(void* at,const void* bytes,size_t n){
 DWORD prot=0,old=0;if(!VirtualProtect(at,n,PAGE_EXECUTE_READWRITE,&prot))return false;
 memcpy(at,bytes,n);bool ok=FlushInstructionCache(GetCurrentProcess(),at,n)!=0;
 return VirtualProtect(at,n,prot,&old)!=0&&ok;
}
inline bool Install(const TsmHost* H,float remaining){
 if(!H||!H->exeBase||!H->allocNear||H->exeSize<0xA9D000||!std::isfinite(remaining)||remaining<0||remaining>1)return false;
 auto* base=H->exeBase;auto* dos=reinterpret_cast<IMAGE_DOS_HEADER*>(base);
 if(dos->e_magic!=IMAGE_DOS_SIGNATURE||dos->e_lfanew<=0||static_cast<size_t>(dos->e_lfanew)+sizeof(IMAGE_NT_HEADERS64)>H->exeSize)return false;
 auto* pe=reinterpret_cast<IMAGE_NT_HEADERS64*>(base+dos->e_lfanew);
 if(pe->Signature!=IMAGE_NT_SIGNATURE||pe->FileHeader.TimeDateStamp!=0x6A3EB6AD||pe->OptionalHeader.SizeOfImage!=0xA9D000)return false;
 const unsigned char expected[]={0x80,0xB9,0x6B,0x17,0x01,0x00,0x00,0x74,0x08,0xF3,0x0F,0x10,0x3D,0xB6,0x44,0x43,0x00};
 if(memcmp(base+0x4D59C9,expected,sizeof(expected))!=0){H->log("TESMIORESEARCH pollution site conflicts or has moved; no code changed");return false;}
 auto* page=H->allocNear(base+0x4D59D2,4096);if(!page)return false;
 const int64_t delta=static_cast<int64_t>(reinterpret_cast<uintptr_t>(page))-static_cast<int64_t>(reinterpret_cast<uintptr_t>(base+0x4D59DA));
 if(delta<INT32_MIN||delta>INT32_MAX){VirtualFree(page,0,MEM_RELEASE);return false;}
 memcpy(page,&remaining,4);DWORD prot=0;if(!VirtualProtect(page,4096,PAGE_READONLY,&prot)){VirtualFree(page,0,MEM_RELEASE);return false;}
 int32_t replacement=static_cast<int32_t>(delta);int32_t original=0x4344B6;
 if(!Write(base+0x4D59D6,&replacement,4)){
  if(Write(base+0x4D59D6,&original,4))VirtualFree(page,0,MEM_RELEASE);
  else H->log("TESMIORESEARCH critical: rollback failed; restart required");return false;
 }
 H->log("TESMIORESEARCH scoped pollution-filter read RVA 0x4D59D2: remaining=%g; shared constants and population effects unchanged",static_cast<double>(remaining));return true;
}
}
