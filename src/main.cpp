#include "main.hpp"
#include "safetyhook.hpp"
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string>
#include <cstdint>
#include <format>
#include "signal.h"
#include <dlfcn.h>
#include <link.h>
#include <iostream>

FakeVSP vsp;

struct ipv4_t{
	uint8_t a1;
	uint8_t a2;
	uint8_t a3;
	uint8_t a4;
};

typedef enum
{
	eSteamValveCDKeyValidationServer = 0,
	eSteamHalfLifeMasterServer = 1,
	eSteamFriendsServer = 2,
	eSteamCSERServer = 3,
	eSteamHalfLife2MasterServer = 4,
	eSteamRDKFMasterServer = 5,
	eMaxServerTypes = 6
} ESteamServerType;

void *GetModuleHandle(const char *name)
{
	void *handle;

	if( name == NULL )
	{
		// hmm, how can this be handled under linux....
		// is it even needed?
		return NULL;
	}

    if( (handle=dlopen(name, RTLD_NOW))==NULL)
    {
            printf("DLOPEN Error:%s\n",dlerror());
            // couldn't open this file
            return NULL;
    }

	// read "man dlopen" for details
	// in short dlopen() inc a ref count
	// so dec the ref count by performing the close
	dlclose(handle);
	return handle;
}

void *GetProcAddress( void* library, const char *name )
{
	return dlsym( library, name );
}

std::string ip2str(in_addr s_addr){
	
	ipv4_t* ip = reinterpret_cast<ipv4_t*>(&s_addr);
	
	return std::format("{}.{}.{}.{}", ip->a1, ip->a2, ip->a3, ip->a4);
}

std::vector<std::string> master_servers;
std::string engine_path;

safetyhook::InlineHook h_SteamFindServersNumServers, h_SteamFindServersIterateServer;

int SteamFindServersNumServers(ESteamServerType eServerType){
	if (eServerType == eSteamHalfLife2MasterServer) {
		return master_servers.size();
	} else {
		return 0;
	}
}

int SteamFindServersIterateServer(ESteamServerType eServerType, unsigned int nServer, char *szIpAddrPort, int szIpAddrPortLen){
	
	if (eServerType != eSteamHalfLife2MasterServer || nServer > master_servers.size()) {
		return -1;
	}
	
	//22
	std::string& server = master_servers[nServer];
	
	if (server.length() + 1 > szIpAddrPortLen){
		return -1;
	}
	
	memcpy(szIpAddrPort, server.c_str(), server.length()+1);
	return 0;
}

bool FakeVSP::Load(uintptr_t interfaceFactory, uintptr_t gameServerFactory){
	
	addrinfo hints{}, *result{};
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_DGRAM;
	
	const in_addr_t main_master = 0x41c840d0;
	
	int status = getaddrinfo("hl2master.steampowered.com", NULL, &hints, &result);
	if (status != 0){
		printf("getaddrinfo status: %s\n", gai_strerror(status));
		raise(SIGTRAP);
		return false;
	}
	
	while (result){
		sockaddr_in* addr = reinterpret_cast<sockaddr_in*>(result->ai_addr);

		if (addr->sin_addr.s_addr == main_master) {
			master_servers.insert(master_servers.begin(), ip2str(addr->sin_addr)+":27011");
		} else {
			master_servers.push_back(ip2str(addr->sin_addr)+":27011");
		}
		
		result = result->ai_next;
	}
	
	freeaddrinfo(result);
	
	// kinda scuffed but works idk linux is weird
	link_map* current_module = reinterpret_cast<link_map*>(dlopen(NULL, RTLD_NOW));
	while (current_module) {
		std::string name(current_module->l_name);
		
		if (name.find("bin/engine_") != std::string::npos){
			engine_path = name.substr(name.find("engine_"));
		}
		
		current_module = current_module->l_next;
	}
	
	void* engine = GetModuleHandle(engine_path.c_str());
	
	//printf("engine handle: %p\n", engine);
	
	void* oSteamFindServersNumServers = GetProcAddress(engine, "SteamFindServersNumServers");
	
	void* oSteamFindServersIterateServer = GetProcAddress(engine, "SteamFindServersIterateServer");

	//printf("SteamFindServersNumServers: %p\n", oSteamFindServersNumServers);
	//printf("SteamFindServersIterateServer: %p\n", oSteamFindServersIterateServer);

	h_SteamFindServersNumServers = safetyhook::create_inline(oSteamFindServersNumServers, &SteamFindServersNumServers);
	h_SteamFindServersIterateServer = safetyhook::create_inline(oSteamFindServersIterateServer, &SteamFindServersIterateServer);
	return true;
}

void FakeVSP::Unload(){
	h_SteamFindServersNumServers = {};
	h_SteamFindServersIterateServer = {};
}

extern "C" __attribute__ ((visibility("default"))) void* CreateInterface(const char* name, int* _) {
	if (strstr(name, "ISERVERPLUGINCALLBACKS")) {
		return &vsp;
	}
	else {
		return 0;
	}
}