/*
The MIT License (MIT)

Copyright (c) 2013-2015 Denilson das Mercês Amorim (aka LINK/2012)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/


#include "modloader.h"

#ifdef MODLOADER
#include <windows.h>
#include <cstdio>
#include <cstdint>

#include "modloader_re3.h"



//
#include "common.h"
#include "Streaming.h"
#include "Directory.h"
#include "CdStream.h"
#include "Timer.h"
#include "FileLoader.h"
#include "main.h"
#include "FileMgr.h"
#include "TxdStore.h"

extern RwUInt32 gGameState;
extern void PlayMovieInWindow(int, const char*);

extern BOOL _gbCdStreamOverlapped;
extern BOOL _gbCdStreamAsync;
extern DWORD _gdwCdStreamFlags;
extern HANDLE gCdStreamSema;
extern Queue gChannelRequestQ;
extern struct CdReadInfo *gpReadInfo;

extern DWORD WINAPI CdStreamThread(LPVOID lpThreadParameter);

static bool CDirectory__FindItem4(void *pThis, const char *name, uint32_t *out1, uint32_t *out2)
{
    return reinterpret_cast<CDirectory*>(pThis)->FindItem(name, *out1, *out2);
}
//


static HMODULE hModLoader;

static modloader_fInitFromRE3 modloader_InitFromRE3;
static modloader_re3_t modloader_re3_share;
static modloader_re3_addr_table_t re3_addr_table;

void ModLoader_Init()
{
    if(!SetEnvironmentVariableA("MODLOADER_RE3", "1")) {
		fprintf(stderr, "Failed to set MODLOADER_RE3=1 (error %d)\n", GetLastError());
		return;
    }

    ::hModLoader = LoadLibraryA("modloader.asi");
    if(hModLoader == NULL) {
        fprintf(stderr, "Failed to load modloader.asi (error %d)\n", GetLastError());
        return;
    }

    modloader_InitFromRE3 = (modloader_fInitFromRE3) GetProcAddress(::hModLoader, "modloader_InitFromRE3");
    if(modloader_InitFromRE3 == NULL) {
        fprintf(stderr, "modloader_InitFromRE3 not found in modloader.asi\n");
        FreeLibrary(::hModLoader);
        hModLoader = NULL;
        return;
    }

    memset(&modloader_re3_share, 0, sizeof(modloader_re3_share));
    modloader_re3_share.size = sizeof(modloader_re3_share);
    modloader_re3_share.re3_version = 0;
    re3_addr_table.size = sizeof(re3_addr_table);
    modloader_re3_share.re3_addr_table = &re3_addr_table;

    re3_addr_table.p_gGameState = reinterpret_cast<int*>(&gGameState);
    re3_addr_table.PlayMovieInWindow = PlayMovieInWindow;
    re3_addr_table.ms_aInfoForModel = CStreaming::ms_aInfoForModel;
    re3_addr_table.uMaxResources = NUMSTREAMINFO;
    re3_addr_table.p_gCdStreamFlags = &_gdwCdStreamFlags;
    re3_addr_table.p_gCdStreamSema = &gCdStreamSema;
    re3_addr_table.p_gChannelRequestQ = &gChannelRequestQ;
    re3_addr_table.p_gpReadInfo = (void**) &gpReadInfo;
    re3_addr_table.p_gbCdStreamOverlapped = &_gbCdStreamOverlapped;
    re3_addr_table.p_gbCdStreamAsync = &_gbCdStreamAsync;
    re3_addr_table.p_ms_pStreamingBuffer = (void**) CStreaming::ms_pStreamingBuffer;
    re3_addr_table.p_ms_streamingBufferSize = (uint32_t*) &CStreaming::ms_streamingBufferSize;
    re3_addr_table.CDirectory__FindItem4 = CDirectory__FindItem4;
    re3_addr_table.CStreaming__LoadCdDirectory0 = &CStreaming::LoadCdDirectory;
    re3_addr_table.CStreaming__LoadCdDirectory2 = &CStreaming::LoadCdDirectory;
    re3_addr_table.CdStreamRead = &CdStreamRead;

    re3_addr_table.CFileMgr_OpenFile = &CFileMgr::OpenFile;
    re3_addr_table.CFileMgr_LoadFile = &CFileMgr::LoadFile;
    re3_addr_table.CTxdStore_LoadTxd = &CTxdStore::LoadTxd;
    re3_addr_table.CFileLoader_LoadAtomicFile2Return = (void*(*)(const char *filename))&CFileLoader::LoadAtomicFile2Return;
    re3_addr_table.InitialiseGame = &InitialiseGame;

    re3_addr_table.LoadCdDirectoryUsingCallbacks = &CStreaming::LoadCdDirectoryUsingCallbacks;

    re3_addr_table.CTimer__Resume = &CTimer::Resume;
    re3_addr_table.CTimer__Suspend = &CTimer::Suspend;
    re3_addr_table.CStreaming__RequestModel = &CStreaming::RequestModel;
    re3_addr_table.CStreaming__RemoveModel = &CStreaming::RemoveModel;
    re3_addr_table.CStreaming__LoadAllRequestedModels = &CStreaming::LoadAllRequestedModels;
    re3_addr_table.CStreaming__FlushChannels = &CStreaming::FlushChannels;
    re3_addr_table.CStreaming__RemoveAllUnusedModels = &CStreaming::RemoveAllUnusedModels;
    re3_addr_table.CStreaming__RemoveLeastUsedModel = &CStreaming::RemoveLeastUsedModel;

    if(!modloader_InitFromRE3(&modloader_re3_share)) {
        FreeLibrary(::hModLoader);
        hModLoader = NULL;
        return;
    }
}

void ModLoader_Shutdown()
{
    if(hModLoader == NULL)
        return;

    modloader_re3_share.Shutdown(&modloader_re3_share);

    FreeLibrary(::hModLoader);
    ::hModLoader = NULL;
}

void ModLoader_Tick()
{
    if(hModLoader == NULL)
        return;

    modloader_re3_share.Tick(&modloader_re3_share);
}

void
ModLoader_PlayMovieInWindow_Logo(int cmdshow, const char *filename)
{
	if(hModLoader == NULL) return;

	if(!modloader_re3_share.callback_table->PlayMovieInWindow_Logo) return modloader_re3_share.re3_addr_table->PlayMovieInWindow(cmdshow, filename);

	return modloader_re3_share.callback_table->PlayMovieInWindow_Logo(cmdshow, filename);
}

void ModLoader_PlayMovieInWindow_GTAtitles(int cmdshow, const char* filename)
{
    if(!modloader_re3_share.callback_table->PlayMovieInWindow_GTAtitles)
		return modloader_re3_share.re3_addr_table->PlayMovieInWindow(cmdshow, filename);

    return modloader_re3_share.callback_table->PlayMovieInWindow_GTAtitles(cmdshow, filename);
}

int32_t
ModLoader_GtaDat(const char* file, const char* mode)
{
    if (!modloader_re3_share.callback_table->OpenFile_GtaDat)
        return modloader_re3_share.re3_addr_table->CFileMgr_OpenFile(file, mode);

    return modloader_re3_share.callback_table->OpenFile_GtaDat(file, mode);
}

int32_t
ModLoader_DefaultDat(const char* file, const char* mode)
{
    if (!modloader_re3_share.callback_table->OpenFile_DefaultDat)
        return modloader_re3_share.re3_addr_table->CFileMgr_OpenFile(file, mode);

    return modloader_re3_share.callback_table->OpenFile_DefaultDat(file, mode);
}

int32_t
ModLoader_CarcolsDat(const char* file, const char* mode)
{
    if (!modloader_re3_share.callback_table->OpenFile_CarcolsDat)
        return modloader_re3_share.re3_addr_table->CFileMgr_OpenFile(file, mode);

    return modloader_re3_share.callback_table->OpenFile_CarcolsDat(file, mode);
}

int32_t ModLoader_PedGrpDat(const char* file, const char* mode)
{
    if (!modloader_re3_share.callback_table->OpenFile_PedGrpDat)
        return modloader_re3_share.re3_addr_table->CFileMgr_OpenFile(file, mode);

    return modloader_re3_share.callback_table->OpenFile_PedGrpDat(file, mode);
}

int32_t ModLoader_CullzoneDat(const char* file, const char* mode)
{
    if (!modloader_re3_share.callback_table->OpenFile_CullzoneDat)
        return modloader_re3_share.re3_addr_table->CFileMgr_OpenFile(file, mode);

    return modloader_re3_share.callback_table->OpenFile_CullzoneDat(file, mode);
}

int32_t ModLoader_MainScm(const char* file, const char* mode)
{
    if (!modloader_re3_share.callback_table->OpenFile_MainScm)
        return modloader_re3_share.re3_addr_table->CFileMgr_OpenFile(file, mode);

    return modloader_re3_share.callback_table->OpenFile_MainScm(file, mode);
}

int32_t ModLoader_WaterproDat(const char* file, const char* mode)
{
    if (!modloader_re3_share.callback_table->OpenFile_WaterproDat)
        return modloader_re3_share.re3_addr_table->CFileMgr_OpenFile(file, mode);

    return modloader_re3_share.callback_table->OpenFile_WaterproDat(file, mode);
}

int32_t ModLoader_PedIfp(const char* file, const char* mode)
{
    if (!modloader_re3_share.callback_table->OpenFile_PedIfp)
        return modloader_re3_share.re3_addr_table->CFileMgr_OpenFile(file, mode);

    return modloader_re3_share.callback_table->OpenFile_PedIfp(file, mode);
}

int ModLoader_FistfiteDat(const char* file, uint8_t* buf, int maxlen, const char* mode)
{
    if (!modloader_re3_share.callback_table->LoadFile_FistfiteDat)
        return modloader_re3_share.re3_addr_table->CFileMgr_LoadFile(file, buf, maxlen, mode);

    return modloader_re3_share.callback_table->LoadFile_FistfiteDat(file, buf, maxlen, mode);
}

int ModLoader_HandlingCfg(const char* file, uint8_t* buf, int maxlen, const char* mode)
{
    if (!modloader_re3_share.callback_table->LoadFile_HandlingCfg)
        return modloader_re3_share.re3_addr_table->CFileMgr_LoadFile(file, buf, maxlen, mode);

    return modloader_re3_share.callback_table->LoadFile_HandlingCfg(file, buf, maxlen, mode);
}

int ModLoader_PedDat(const char* file, uint8_t* buf, int maxlen, const char* mode)
{
    if (!modloader_re3_share.callback_table->LoadFile_PedDat)
        return modloader_re3_share.re3_addr_table->CFileMgr_LoadFile(file, buf, maxlen, mode);

    return modloader_re3_share.callback_table->LoadFile_PedDat(file, buf, maxlen, mode);
}

int ModLoader_ObjectDat(const char* file, uint8_t* buf, int maxlen, const char* mode)
{
    if (!modloader_re3_share.callback_table->LoadFile_ObjectDat)
        return modloader_re3_share.re3_addr_table->CFileMgr_LoadFile(file, buf, maxlen, mode);

    return modloader_re3_share.callback_table->LoadFile_ObjectDat(file, buf, maxlen, mode);
}

int ModLoader_PedStatsDat(const char* file, uint8_t* buf, int maxlen, const char* mode)
{
    if (!modloader_re3_share.callback_table->LoadFile_PedStatsDat)
        return modloader_re3_share.re3_addr_table->CFileMgr_LoadFile(file, buf, maxlen, mode);

    return modloader_re3_share.callback_table->LoadFile_PedStatsDat(file, buf, maxlen, mode);
}

int ModLoader_WeaponDat(const char* file, uint8_t* buf, int maxlen, const char* mode)
{
    if (!modloader_re3_share.callback_table->LoadFile_WeaponDat)
        return modloader_re3_share.re3_addr_table->CFileMgr_LoadFile(file, buf, maxlen, mode);

    return modloader_re3_share.callback_table->LoadFile_WeaponDat(file, buf, maxlen, mode);
}

int ModLoader_ParticleCfg(const char* file, uint8_t* buf, int maxlen, const char* mode)
{
    if (!modloader_re3_share.callback_table->LoadFile_ParticleCfg)
        return modloader_re3_share.re3_addr_table->CFileMgr_LoadFile(file, buf, maxlen, mode);

    return modloader_re3_share.callback_table->LoadFile_ParticleCfg(file, buf, maxlen, mode);
}

int ModLoader_SurfaceDat(const char* file, uint8_t* buf, int maxlen, const char* mode)
{
    if (!modloader_re3_share.callback_table->LoadFile_SurfaceDat)
        return modloader_re3_share.re3_addr_table->CFileMgr_LoadFile(file, buf, maxlen, mode);

    return modloader_re3_share.callback_table->LoadFile_SurfaceDat(file, buf, maxlen, mode);
}

int ModLoader_TimecycleDat(const char* file, uint8_t* buf, int maxlen, const char* mode)
{
    if (!modloader_re3_share.callback_table->LoadFile_TimecycleDat)
        return modloader_re3_share.re3_addr_table->CFileMgr_LoadFile(file, buf, maxlen, mode);

    return modloader_re3_share.callback_table->LoadFile_TimecycleDat(file, buf, maxlen, mode);
}

int ModLoader_FlightDat(const char* file, uint8_t* buf, int maxlen, const char* mode)
{
    if (strstr(file, "flight.dat")) {
        if (!modloader_re3_share.callback_table->LoadFile_FlightDat)
            return modloader_re3_share.re3_addr_table->CFileMgr_LoadFile(file, buf, maxlen, mode);

        return modloader_re3_share.callback_table->LoadFile_FlightDat(file, buf, maxlen, mode);
    } else if (strstr(file, "flight2.dat")) {
        if (!modloader_re3_share.callback_table->LoadFile_Flight2Dat)
            return modloader_re3_share.re3_addr_table->CFileMgr_LoadFile(file, buf, maxlen, mode);

        return modloader_re3_share.callback_table->LoadFile_Flight2Dat(file, buf, maxlen, mode);
    } else if (strstr(file, "flight3.dat")) {
        if (!modloader_re3_share.callback_table->LoadFile_Flight3Dat)
            return modloader_re3_share.re3_addr_table->CFileMgr_LoadFile(file, buf, maxlen, mode);

        return modloader_re3_share.callback_table->LoadFile_Flight3Dat(file, buf, maxlen, mode);
    } else { // flight4.dat
        if (!modloader_re3_share.callback_table->LoadFile_Flight4Dat)
            return modloader_re3_share.re3_addr_table->CFileMgr_LoadFile(file, buf, maxlen, mode);

        return modloader_re3_share.callback_table->LoadFile_Flight4Dat(file, buf, maxlen, mode);
    }
}

int ModLoader_TracksDat(const char* file, uint8_t* buf, int maxlen, const char* mode)
{
    if (strstr(file, "tracks.dat")) {
        if (!modloader_re3_share.callback_table->LoadFile_TracksDat)
            return modloader_re3_share.re3_addr_table->CFileMgr_LoadFile(file, buf, maxlen, mode);

        return modloader_re3_share.callback_table->LoadFile_TracksDat(file, buf, maxlen, mode);
    } else { // tracks2.dat
        if (!modloader_re3_share.callback_table->LoadFile_Tracks2Dat)
            return modloader_re3_share.re3_addr_table->CFileMgr_LoadFile(file, buf, maxlen, mode);

        return modloader_re3_share.callback_table->LoadFile_Tracks2Dat(file, buf, maxlen, mode);
    }
}

bool ModLoader_FontsTxd(int slot, const char* file)
{
    if (strstr(file, "FONTS.TXD")) {
        if (!modloader_re3_share.callback_table->LoadTxd_FontsTxd)
            return modloader_re3_share.re3_addr_table->CTxdStore_LoadTxd(slot, file);

        return modloader_re3_share.callback_table->LoadTxd_FontsTxd(slot, file);
    } else if (strstr(file, "FONTS_P.TXD")) {
        if (!modloader_re3_share.callback_table->LoadTxd_FontsPTxd)
            return modloader_re3_share.re3_addr_table->CTxdStore_LoadTxd(slot, file);

        return modloader_re3_share.callback_table->LoadTxd_FontsPTxd(slot, file);
    } else if (strstr(file, "FONTS_R.TXD")) {
        if (!modloader_re3_share.callback_table->LoadTxd_FontsRTxd)
            return modloader_re3_share.re3_addr_table->CTxdStore_LoadTxd(slot, file);

        return modloader_re3_share.callback_table->LoadTxd_FontsRTxd(slot, file);
    } else { // FONTS_J.TXD
        if (!modloader_re3_share.callback_table->LoadTxd_FontsJTxd)
            return modloader_re3_share.re3_addr_table->CTxdStore_LoadTxd(slot, file);

        return modloader_re3_share.callback_table->LoadTxd_FontsJTxd(slot, file);
    }
}

bool ModLoader_FrontendTxd(int slot, const char* file)
{
    if (!modloader_re3_share.callback_table->LoadTxd_FrontendTxd)
        return modloader_re3_share.re3_addr_table->CTxdStore_LoadTxd(slot, file);

    return modloader_re3_share.callback_table->LoadTxd_FrontendTxd(slot, file);
}

bool ModLoader_HudTxd(int slot, const char* file)
{
    if (!modloader_re3_share.callback_table->LoadTxd_HudTxd)
        return modloader_re3_share.re3_addr_table->CTxdStore_LoadTxd(slot, file);

    return modloader_re3_share.callback_table->LoadTxd_HudTxd(slot, file);
}

bool ModLoader_MenuTxd(int slot, const char* file)
{
    if (!modloader_re3_share.callback_table->LoadTxd_MenuTxd)
        return modloader_re3_share.re3_addr_table->CTxdStore_LoadTxd(slot, file);

    return modloader_re3_share.callback_table->LoadTxd_MenuTxd(slot, file);
}

bool ModLoader_ParticleTxd(int slot, const char* file)
{
    if (!modloader_re3_share.callback_table->LoadTxd_ParticleTxd)
        return modloader_re3_share.re3_addr_table->CTxdStore_LoadTxd(slot, file);

    return modloader_re3_share.callback_table->LoadTxd_ParticleTxd(slot, file);
}

void* ModLoader_ArrowDff(const char* file)
{
    if (!modloader_re3_share.callback_table->LoadAtomic2Return_ArrowDff)
        return modloader_re3_share.re3_addr_table->CFileLoader_LoadAtomicFile2Return(file);

    return modloader_re3_share.callback_table->LoadAtomic2Return_ArrowDff(file);
}

void* ModLoader_ZonecylbDff(const char* file)
{
    if (!modloader_re3_share.callback_table->LoadAtomic2Return_ZonecylbDff)
        return modloader_re3_share.re3_addr_table->CFileLoader_LoadAtomicFile2Return(file);

    return modloader_re3_share.callback_table->LoadAtomic2Return_ZonecylbDff(file);
}

void ModLoader_InitialiseGame()
{
    if (!modloader_re3_share.callback_table->InitialiseGame)
        return modloader_re3_share.re3_addr_table->InitialiseGame();

    return modloader_re3_share.callback_table->InitialiseGame();
}


bool
ModLoader_CdStreamThread()
{
	if(modloader_re3_share.callback_table->CdStreamThread) {
        modloader_re3_share.callback_table->CdStreamThread();
        return true;
    }
    return false; 
}


void*
ModLoader_AcquireNextModelFileHandle()
{
	if(modloader_re3_share.callback_table->AcquireNextModelFileHandle)
		return modloader_re3_share.callback_table->AcquireNextModelFileHandle();
    return INVALID_HANDLE_VALUE;
}

void
ModLoader_LoadCdDirectory()
{
	if(modloader_re3_share.callback_table->LoadCdDirectory0)
        return modloader_re3_share.callback_table->LoadCdDirectory0();
	return modloader_re3_share.re3_addr_table->CStreaming__LoadCdDirectory0();
}

void
ModLoader_FetchCdDirectory(const char *filename, int cd_index)
{
	if(modloader_re3_share.callback_table->FetchCdDirectory)
        return modloader_re3_share.callback_table->FetchCdDirectory(filename, cd_index);
	return modloader_re3_share.re3_addr_table->CStreaming__LoadCdDirectory2(filename, cd_index);
}

void
ModLoader_RegisterNextModelRead(int resource_id)
{
	if(modloader_re3_share.callback_table->RegisterNextModelRead)
        return modloader_re3_share.callback_table->RegisterNextModelRead(resource_id);
}

int32_t
ModLoader_CdStreamRead(int32_t channel, void *buffer, uint32_t offset, uint32_t size)
{
	if(modloader_re3_share.callback_table->CdStreamThread)
        return modloader_re3_share.callback_table->CdStreamRead(channel, buffer, offset, size);
	return modloader_re3_share.re3_addr_table->CdStreamRead(channel, buffer, offset, size);
}

void
ModLoader_OnRequestSpecialModel(uint32_t model_id, const char* model_name, uint32_t pos, uint32_t size)
{
	if(modloader_re3_share.callback_table->OnRequestSpecialModel)
		return modloader_re3_share.callback_table->OnRequestSpecialModel(model_id, model_name, pos, size);
}

const char*
ModLoader_GetCdStreamPath_Unsafe(const char *filepath)
{
    if(modloader_re3_share.callback_table->GetCdStreamPath_Unsafe)
        return modloader_re3_share.callback_table->GetCdStreamPath_Unsafe(filepath);
    return filepath;
}

const char*
ModLoader_GetCdDirectoryPath_Unsafe(const char *filepath)
{
	if(modloader_re3_share.callback_table->GetCdDirectoryPath_Unsafe)
        return modloader_re3_share.callback_table->GetCdDirectoryPath_Unsafe(filepath);
	return filepath;
}

const char *
ModLoader_RegisterAndGetColFile_Unsafe(const char *filepath)
{
	if(modloader_re3_share.callback_table->RegisterAndGetColFile_Unsafe)
        return modloader_re3_share.callback_table->RegisterAndGetColFile_Unsafe(filepath);
	return filepath;
}

const char *
ModLoader_RegisterAndGetAtomicFile_Unsafe(const char *filepath)
{
	if(modloader_re3_share.callback_table->RegisterAndGetAtomicFile_Unsafe)
		return modloader_re3_share.callback_table->RegisterAndGetAtomicFile_Unsafe(filepath);
	return filepath;
}

const char *
ModLoader_RegisterAndGetClumpFile_Unsafe(const char *filepath)
{
	if(modloader_re3_share.callback_table->RegisterAndGetClumpFile_Unsafe)
		return modloader_re3_share.callback_table->RegisterAndGetClumpFile_Unsafe(filepath);
	return filepath;
}

const char *
ModLoader_RegisterAndGetTexDiction_Unsafe(const char *filepath)
{
	if(modloader_re3_share.callback_table->RegisterAndGetTexDiction_Unsafe)
		return modloader_re3_share.callback_table->RegisterAndGetTexDiction_Unsafe(filepath);
	return filepath;
}

const char*
ModLoader_RegisterAndGetIdeFile_Unsafe(const char* filepath)
{
    if (modloader_re3_share.callback_table->RegisterAndGetIdeFile_Unsafe)
        return modloader_re3_share.callback_table->RegisterAndGetIdeFile_Unsafe(filepath);
    return filepath;
}

const char*
ModLoader_RegisterAndGetIplFile_Unsafe(const char* filepath)
{
    if (modloader_re3_share.callback_table->RegisterAndGetIplFile_Unsafe)
        return modloader_re3_share.callback_table->RegisterAndGetIplFile_Unsafe(filepath);
    return filepath;
}

const char* ModLoader_RegisterAndGetGxtFile_Unsafe(const char* filepath)
{
    if (modloader_re3_share.callback_table->RegisterAndGetGxtFile_Unsafe)
        return modloader_re3_share.callback_table->RegisterAndGetGxtFile_Unsafe(filepath);
    return filepath;
}

const char* ModLoader_RegisterAndGetPlayerBmpFile_Unsafe(const char* filepath)
{
    if (modloader_re3_share.callback_table->RegisterAndGetPlayerBmpFile_Unsafe)
        return modloader_re3_share.callback_table->RegisterAndGetPlayerBmpFile_Unsafe(filepath);
    return filepath;
}

const char* ModLoader_RegisterAndGetSplashFile_Unsafe(const char* filepath)
{
    if (modloader_re3_share.callback_table->RegisterAndGetSplashFile_Unsafe)
        return modloader_re3_share.callback_table->RegisterAndGetSplashFile_Unsafe(filepath);
    return filepath;
}
#endif