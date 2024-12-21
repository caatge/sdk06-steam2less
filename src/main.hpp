#pragma once
// Absolutely real VSP
#include <cstdint>
#include <cstring>
#include <cstdio>

class FakeVSP
{
public:

	virtual bool Load(uintptr_t interfaceFactory, uintptr_t gameServerFactory);

	virtual void			Unload(void);
	virtual void			Pause(void) {}
	virtual void			UnPause(void) {}

	virtual const char* GetPluginDescription(void) {
		return "class dumper";
	}

	virtual void			LevelInit(char const* pMapName) {}
	virtual void			ServerActivate(uintptr_t pEdictList, int edictCount, int clientMax) {}
	virtual void			GameFrame(bool simulating) {}
	virtual void			LevelShutdown(void) {}
	virtual void			ClientActive(uintptr_t pEntity) {}
	virtual void			ClientDisconnect(uintptr_t pEntity) {}
	virtual void			ClientPutInServer(uintptr_t pEntity, char const* playername) {}
	virtual void			SetCommandClient(int index) {}
	virtual void			ClientSettingsChanged(uintptr_t pEdict) {}
	virtual int	ClientConnect(bool* bAllowConnect, uintptr_t pEntity, const char* pszName, const char* pszAddress, char* reject, int maxrejectlen) {
		return 0;
	}

	// grrr
	virtual int ClientCommand(size_t a1, size_t a2) {
		return 0;
	}

	virtual int	NetworkIDValidated(const char* pszUserName, const char* pszNetworkID) {
		return 0;
	}
	virtual void			OnQueryCvarValueFinished(int iCookie, uintptr_t pPlayerEntity, int eStatus, const char* pCvarName, const char* pCvarValue) {}
	virtual void			OnEdictAllocated(uintptr_t edict) {}
	virtual void			OnEdictFreed(const uintptr_t edict) {}
};

