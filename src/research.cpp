#include "adapter.h"
#include "research_native.h"
namespace Research {
using namespace Adapter;
inline double speed=1,duration=1,quality=.899999,height=33.8,pollution=.775;
inline std::map<std::string,double> typeSpeed;
inline bool started=false;
bool Accepts(const std::string& p){const std::string suffix="media_soviet/research/research.ini";return p.size()>=suffix.size()&&p.substr(p.size()-suffix.size())==suffix;}
std::string Transform(const std::string&,const std::string& bytes){
 auto lines=Lines(bytes);std::vector<std::pair<std::string,size_t>> blocks;
 for(size_t i=0;i<lines.size();++i){std::istringstream in(lines[i]);std::string cmd,id;in>>cmd>>id;if(cmd=="$RESEARCH")blocks.emplace_back(id,i);}
 bool changed=false;
 for(size_t i=0;i<blocks.size();++i){
  const auto& id=blocks[i].first;size_t a=blocks[i].second,b=i+1<blocks.size()?blocks[i+1].second:lines.size();
  std::string type="technical";
  for(size_t j=a;j<b;++j){std::istringstream in(lines[j]);std::string cmd;in>>cmd;if(cmd=="$TYPE_MEDICAL")type="medical";if(cmd=="$TYPE_SOVIET")type="soviet";}
  double factor=speed*typeSpeed.at(type)*Setting("research."+id,"speed_multiplier",1);
  for(size_t j=a;j<b;++j){std::istringstream in(lines[j]);std::string cmd;double value;if(!(in>>cmd>>value))continue;
   double result=value;
   if(cmd=="$COST")result=std::max(1.0,std::round(value/factor));
   else if(cmd=="$LOCK_AFTER_DAYS")result=std::max(1.0,std::round(value*duration));
   else if(cmd=="$UNLOCK_BUILDING_RESIDENTIAL_QUALITY")result=quality;
   else if(cmd=="$UNLOCK_BUILDING_RESIDENTIAL_TALL")result=height;
   if(result!=value){std::string tail;std::getline(in,tail);lines[j]=cmd+" "+Num(result)+tail;changed=true;}
  }
 }
 return changed?Join(lines):bytes;
}
int Init(const TsmHost* h,TsmPluginInfo* info){
 if(!info||!ValidHost(h))return 1;info->name="TESMIORESEARCH";info->version="0.3.1";
 try{
  Setup(h,info->name);enabled=Toggle("research","enabled",1);if(!enabled){H->log("TESMIORESEARCH disabled; no hooks installed");return 0;}
  speed=Setting("research","speed_multiplier",1);
  duration=Setting("effects","policy_duration_multiplier",1);
  quality=Setting("effects","housing_quality_threshold",.899999,.1,1);
  height=Setting("effects","housing_height_threshold",33.8,1,500);
  pollution=Setting("effects","pollution_filter_remaining",.775,0,1);
  for(auto type:{"technical","soviet","medical"})typeSpeed[type]=Setting(type,"speed_multiplier",1);
  accepts=Accepts;transform=Transform;asset={};
  H->log("TESMIORESEARCH original research only: speed=%g policy duration=%g pollution remaining=%g; no expanded tree, UI or completion hooks",speed,duration,pollution);
  return 0;
 }catch(const std::exception& e){enabled=false;H->log("TESMIORESEARCH configuration rejected: %s",e.what());return 1;}
}
int Start(){
 if(!enabled||started)return 0;started=true;Adapter::Start();
 if(static_cast<float>(pollution)!=.775f){
  if(!NativeResearch::ExactExe(H)||!NativeResearch::Install(H,static_cast<float>(pollution)))H->log("TESMIORESEARCH pollution effect refused; original coefficient retained");
 }
 return 0;
}
}
extern "C" __declspec(dllexport) unsigned TsmPluginApiVersion(){return TSM_API_VERSION;}
extern "C" __declspec(dllexport) int TsmPluginInit(const TsmHost* h,TsmPluginInfo* i){return Research::Init(h,i);}
extern "C" __declspec(dllexport) int TsmPluginStart(){return Research::Start();}
