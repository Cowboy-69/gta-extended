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

#include "modloader_reVC.h"



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
extern struct CdReadInfo* gpReadInfo;

extern DWORD WINAPI CdStreamThread(LPVOID lpThreadParameter);

static bool CDirectory__FindItem4(void* pThis, const char* name, uint32_t* out1, uint32_t* out2)
{
    return reinterpret_cast<CDirectory*>(pThis)->FindItem(name, *out1, *out2);
}
//


static HMODULE hModLoader;

static modloader_fInitFromREVC modloader_InitFromREVC;
static modloader_reVC_t modloader_reVC_share;
static modloader_reVC_addr_table_t reVC_addr_table;

void ModLoader_Init()
{
    if (!SetEnvironmentVariableA("MODLOADER_REVC", "1")) {
        fprintf(stderr, "Failed to set MODLOADER_REVC=1 (error %d)\n", GetLastError());
        return;
    }

    ::hModLoader = LoadLibraryA("modloader.asi");
    if (hModLoader == NULL) {
        fprintf(stderr, "Failed to load modloader.asi (error %d)\n", GetLastError());
        return;
    }

    modloader_InitFromREVC = (modloader_fInitFromREVC)GetProcAddress(::hModLoader, "modloader_InitFromREVC");
    if (modloader_InitFromREVC == NULL) {
        fprintf(stderr, "modloader_InitFromREVC not found in modloader.asi\n");
        FreeLibrary(::hModLoader);
        hModLoader = NULL;
        return;
    }

    memset(&modloader_reVC_share, 0, sizeof(modloader_reVC_share));
    modloader_reVC_share.size = sizeof(modloader_reVC_share);
    modloader_reVC_share.reVC_version = 0;
    reVC_addr_table.size = sizeof(reVC_addr_table);
    modloader_reVC_share.reVC_addr_table = &reVC_addr_table;

    reVC_addr_table.p_gGameState = reinterpret_cast<int*>(&gGameState);
    reVC_addr_table.PlayMovieInWindow = PlayMovieInWindow;
    reVC_addr_table.ms_aInfoForModel = CStreaming::ms_aInfoForModel;
    reVC_addr_table.uMaxResources = NUMSTREAMINFO;
    reVC_addr_table.p_gCdStreamFlags = &_gdwCdStreamFlags;
    reVC_addr_table.p_gCdStreamSema = &gCdStreamSema;
    reVC_addr_table.p_gChannelRequestQ = &gChannelRequestQ;
    reVC_addr_table.p_gpReadInfo = (void**)&gpReadInfo;
    reVC_addr_table.p_gbCdStreamOverlapped = &_gbCdStreamOverlapped;
    reVC_addr_table.p_gbCdStreamAsync = &_gbCdStreamAsync;
    reVC_addr_table.p_ms_pStreamingBuffer = (void**)CStreaming::ms_pStreamingBuffer;
    reVC_addr_table.p_ms_streamingBufferSize = (uint32_t*)&CStreaming::ms_streamingBufferSize;
    reVC_addr_table.CDirectory__FindItem4 = CDirectory__FindItem4;
    reVC_addr_table.CStreaming__LoadCdDirectory0 = &CStreaming::LoadCdDirectory;
    reVC_addr_table.CStreaming__LoadCdDirectory2 = &CStreaming::LoadCdDirectory;
    reVC_addr_table.CdStreamRead = &CdStreamRead;

    reVC_addr_table.CFileMgr_OpenFile = &CFileMgr::OpenFile;
    reVC_addr_table.CFileMgr_LoadFile = &CFileMgr::LoadFile;
    reVC_addr_table.CTxdStore_LoadTxd = &CTxdStore::LoadTxd;
    reVC_addr_table.CFileLoader_LoadAtomicFile2Return = (void* (*)(const char* filename)) &CFileLoader::LoadAtomicFile2Return;
    reVC_addr_table.RwStreamOpen = (void* (*)(int32_t type, int32_t accessType, const char* pData)) &RwStreamOpen;
    reVC_addr_table.InitialiseGame = &InitialiseGame;

    reVC_addr_table.LoadCdDirectoryUsingCallbacks = &CStreaming::LoadCdDirectoryUsingCallbacks;

    reVC_addr_table.CTimer__Resume = &CTimer::Resume;
    reVC_addr_table.CTimer__Suspend = &CTimer::Suspend;
    reVC_addr_table.CStreaming__RequestModel = &CStreaming::RequestModel;
    reVC_addr_table.CStreaming__RemoveModel = &CStreaming::RemoveModel;
    reVC_addr_table.CStreaming__LoadAllRequestedModels = &CStreaming::LoadAllRequestedModels;
    reVC_addr_table.CStreaming__FlushChannels = &CStreaming::FlushChannels;
    reVC_addr_table.CStreaming__RemoveAllUnusedModels = &CStreaming::RemoveAllUnusedModels;
    reVC_addr_table.CStreaming__RemoveLeastUsedModel2 = &CStreaming::RemoveLeastUsedModel;

    if (!modloader_InitFromREVC(&modloader_reVC_share)) {
        FreeLibrary(::hModLoader);
        hModLoader = NULL;
        return;
    }
}

void ModLoader_Shutdown()
{
    if (hModLoader == NULL)
        return;

    modloader_reVC_share.Shutdown(&modloader_reVC_share);

    FreeLibrary(::hModLoader);
    ::hModLoader = NULL;
}

void ModLoader_Tick()
{
    if (hModLoader == NULL)
        return;

    modloader_reVC_share.Tick(&modloader_reVC_share);
}

void
ModLoader_PlayMovieInWindow_Logo(int cmdshow, const char* filename)
{
    if (hModLoader == NULL) return;

    if (!modloader_reVC_share.callback_table->PlayMovieInWindow_Logo) return modloader_reVC_share.reVC_addr_table->PlayMovieInWindow(cmdshow, filename);

    return modloader_reVC_share.callback_table->PlayMovieInWindow_Logo(cmdshow, filename);
}

void ModLoader_PlayMovieInWindow_GTAtitles(int cmdshow, const char* filename)
{
    if (!modloader_reVC_share.callback_table->PlayMovieInWindow_GTAtitles)
        return modloader_reVC_share.reVC_addr_table->PlayMovieInWindow(cmdshow, filename);

    return modloader_reVC_share.callback_table->PlayMovieInWindow_GTAtitles(cmdshow, filename);
}

int32_t
ModLoader_GtaDat(const char* file, const char* mode)
{
    if (!modloader_reVC_share.callback_table->OpenFile_GtaDat)
        return modloader_reVC_share.reVC_addr_table->CFileMgr_OpenFile(file, mode);

    return modloader_reVC_share.callback_table->OpenFile_GtaDat(file, mode);
}

int32_t
ModLoader_DefaultDat(const char* file, const char* mode)
{
    if (!modloader_reVC_share.callback_table->OpenFile_DefaultDat)
        return modloader_reVC_share.reVC_addr_table->CFileMgr_OpenFile(file, mode);

    return modloader_reVC_share.callback_table->OpenFile_DefaultDat(file, mode);
}

int32_t
ModLoader_CarcolsDat(const char* file, const char* mode)
{
    if (!modloader_reVC_share.callback_table->OpenFile_CarcolsDat)
        return modloader_reVC_share.reVC_addr_table->CFileMgr_OpenFile(file, mode);

    return modloader_reVC_share.callback_table->OpenFile_CarcolsDat(file, mode);
}

int32_t ModLoader_PedGrpDat(const char* file, const char* mode)
{
    if (!modloader_reVC_share.callback_table->OpenFile_PedGrpDat)
        return modloader_reVC_share.reVC_addr_table->CFileMgr_OpenFile(file, mode);

    return modloader_reVC_share.callback_table->OpenFile_PedGrpDat(file, mode);
}

int32_t ModLoader_MainScm(const char* file, const char* mode)
{
    if (!modloader_reVC_share.callback_table->OpenFile_MainScm)
        return modloader_reVC_share.reVC_addr_table->CFileMgr_OpenFile(file, mode);

    return modloader_reVC_share.callback_table->OpenFile_MainScm(file, mode);
}

int32_t ModLoader_WaterproDat(const char* file, const char* mode)
{
    if (!modloader_reVC_share.callback_table->OpenFile_WaterproDat)
        return modloader_reVC_share.reVC_addr_table->CFileMgr_OpenFile(file, mode);

    return modloader_reVC_share.callback_table->OpenFile_WaterproDat(file, mode);
}

int ModLoader_FistfiteDat(const char* file, uint8_t* buf, int maxlen, const char* mode)
{
    if (!modloader_reVC_share.callback_table->LoadFile_FistfiteDat)
        return modloader_reVC_share.reVC_addr_table->CFileMgr_LoadFile(file, buf, maxlen, mode);

    return modloader_reVC_share.callback_table->LoadFile_FistfiteDat(file, buf, maxlen, mode);
}

int ModLoader_HandlingCfg(const char* file, uint8_t* buf, int maxlen, const char* mode)
{
    if (!modloader_reVC_share.callback_table->LoadFile_HandlingCfg)
        return modloader_reVC_share.reVC_addr_table->CFileMgr_LoadFile(file, buf, maxlen, mode);

    return modloader_reVC_share.callback_table->LoadFile_HandlingCfg(file, buf, maxlen, mode);
}

int ModLoader_PedDat(const char* file, uint8_t* buf, int maxlen, const char* mode)
{
    if (!modloader_reVC_share.callback_table->LoadFile_PedDat)
        return modloader_reVC_share.reVC_addr_table->CFileMgr_LoadFile(file, buf, maxlen, mode);

    return modloader_reVC_share.callback_table->LoadFile_PedDat(file, buf, maxlen, mode);
}

int ModLoader_ObjectDat(const char* file, uint8_t* buf, int maxlen, const char* mode)
{
    if (!modloader_reVC_share.callback_table->LoadFile_ObjectDat)
        return modloader_reVC_share.reVC_addr_table->CFileMgr_LoadFile(file, buf, maxlen, mode);

    return modloader_reVC_share.callback_table->LoadFile_ObjectDat(file, buf, maxlen, mode);
}

int ModLoader_PedStatsDat(const char* file, uint8_t* buf, int maxlen, const char* mode)
{
    if (!modloader_reVC_share.callback_table->LoadFile_PedStatsDat)
        return modloader_reVC_share.reVC_addr_table->CFileMgr_LoadFile(file, buf, maxlen, mode);

    return modloader_reVC_share.callback_table->LoadFile_PedStatsDat(file, buf, maxlen, mode);
}

int ModLoader_WeaponDat(const char* file, uint8_t* buf, int maxlen, const char* mode)
{
    if (!modloader_reVC_share.callback_table->LoadFile_WeaponDat)
        return modloader_reVC_share.reVC_addr_table->CFileMgr_LoadFile(file, buf, maxlen, mode);

    return modloader_reVC_share.callback_table->LoadFile_WeaponDat(file, buf, maxlen, mode);
}

int ModLoader_ParticleCfg(const char* file, uint8_t* buf, int maxlen, const char* mode)
{
    if (!modloader_reVC_share.callback_table->LoadFile_ParticleCfg)
        return modloader_reVC_share.reVC_addr_table->CFileMgr_LoadFile(file, buf, maxlen, mode);

    return modloader_reVC_share.callback_table->LoadFile_ParticleCfg(file, buf, maxlen, mode);
}

int ModLoader_SurfaceDat(const char* file, uint8_t* buf, int maxlen, const char* mode)
{
    if (!modloader_reVC_share.callback_table->LoadFile_SurfaceDat)
        return modloader_reVC_share.reVC_addr_table->CFileMgr_LoadFile(file, buf, maxlen, mode);

    return modloader_reVC_share.callback_table->LoadFile_SurfaceDat(file, buf, maxlen, mode);
}

int ModLoader_TimecycleDat(const char* file, uint8_t* buf, int maxlen, const char* mode)
{
    if (!modloader_reVC_share.callback_table->LoadFile_TimecycleDat)
        return modloader_reVC_share.reVC_addr_table->CFileMgr_LoadFile(file, buf, maxlen, mode);

    return modloader_reVC_share.callback_table->LoadFile_TimecycleDat(file, buf, maxlen, mode);
}

int ModLoader_FlightDat(const char* file, uint8_t* buf, int maxlen, const char* mode)
{
    if (strstr(file, "flight.dat")) {
        if (!modloader_reVC_share.callback_table->LoadFile_FlightDat)
            return modloader_reVC_share.reVC_addr_table->CFileMgr_LoadFile(file, buf, maxlen, mode);

        return modloader_reVC_share.callback_table->LoadFile_FlightDat(file, buf, maxlen, mode);
    } else if (strstr(file, "flight2.dat")) {
        if (!modloader_reVC_share.callback_table->LoadFile_Flight2Dat)
            return modloader_reVC_share.reVC_addr_table->CFileMgr_LoadFile(file, buf, maxlen, mode);

        return modloader_reVC_share.callback_table->LoadFile_Flight2Dat(file, buf, maxlen, mode);
    } else {
        // flight3.dat
        if (!modloader_reVC_share.callback_table->LoadFile_Flight3Dat)
            return modloader_reVC_share.reVC_addr_table->CFileMgr_LoadFile(file, buf, maxlen, mode);

        return modloader_reVC_share.callback_table->LoadFile_Flight3Dat(file, buf, maxlen, mode);
    }
}

int ModLoader_TracksDat(const char* file, uint8_t* buf, int maxlen, const char* mode)
{
    if (strstr(file, "tracks.dat")) {
        if (!modloader_reVC_share.callback_table->LoadFile_TracksDat)
            return modloader_reVC_share.reVC_addr_table->CFileMgr_LoadFile(file, buf, maxlen, mode);

        return modloader_reVC_share.callback_table->LoadFile_TracksDat(file, buf, maxlen, mode);
    }
    else { // tracks2.dat
        if (!modloader_reVC_share.callback_table->LoadFile_Tracks2Dat)
            return modloader_reVC_share.reVC_addr_table->CFileMgr_LoadFile(file, buf, maxlen, mode);

        return modloader_reVC_share.callback_table->LoadFile_Tracks2Dat(file, buf, maxlen, mode);
    }
}

bool ModLoader_FontsTxd(int slot, const char* file)
{
    if (strstr(file, "FONTS.TXD")) {
        if (!modloader_reVC_share.callback_table->LoadTxd_FontsTxd)
            return modloader_reVC_share.reVC_addr_table->CTxdStore_LoadTxd(slot, file);

        return modloader_reVC_share.callback_table->LoadTxd_FontsTxd(slot, file);
    }
    else if (strstr(file, "FONTS_P.TXD")) {
        if (!modloader_reVC_share.callback_table->LoadTxd_FontsPTxd)
            return modloader_reVC_share.reVC_addr_table->CTxdStore_LoadTxd(slot, file);

        return modloader_reVC_share.callback_table->LoadTxd_FontsPTxd(slot, file);
    }
    else if (strstr(file, "FONTS_R.TXD")) {
        if (!modloader_reVC_share.callback_table->LoadTxd_FontsRTxd)
            return modloader_reVC_share.reVC_addr_table->CTxdStore_LoadTxd(slot, file);

        return modloader_reVC_share.callback_table->LoadTxd_FontsRTxd(slot, file);
    }
    else { // FONTS_J.TXD
        if (!modloader_reVC_share.callback_table->LoadTxd_FontsJTxd)
            return modloader_reVC_share.reVC_addr_table->CTxdStore_LoadTxd(slot, file);

        return modloader_reVC_share.callback_table->LoadTxd_FontsJTxd(slot, file);
    }
}

bool ModLoader_Fronten1Txd(int slot, const char* file)
{
    if (!modloader_reVC_share.callback_table->LoadTxd_Fronten1Txd)
        return modloader_reVC_share.reVC_addr_table->CTxdStore_LoadTxd(slot, file);

    return modloader_reVC_share.callback_table->LoadTxd_Fronten1Txd(slot, file);
}

bool ModLoader_HudTxd(int slot, const char* file)
{
    if (!modloader_reVC_share.callback_table->LoadTxd_HudTxd)
        return modloader_reVC_share.reVC_addr_table->CTxdStore_LoadTxd(slot, file);

    return modloader_reVC_share.callback_table->LoadTxd_HudTxd(slot, file);
}

bool ModLoader_Fronten2Txd(int slot, const char* file)
{
    if (!modloader_reVC_share.callback_table->LoadTxd_Fronten2Txd)
        return modloader_reVC_share.reVC_addr_table->CTxdStore_LoadTxd(slot, file);

    return modloader_reVC_share.callback_table->LoadTxd_Fronten2Txd(slot, file);
}

bool ModLoader_ParticleTxd(int slot, const char* file)
{
    if (!modloader_reVC_share.callback_table->LoadTxd_ParticleTxd)
        return modloader_reVC_share.reVC_addr_table->CTxdStore_LoadTxd(slot, file);

    return modloader_reVC_share.callback_table->LoadTxd_ParticleTxd(slot, file);
}

void* ModLoader_ArrowDff(const char* file)
{
    if (!modloader_reVC_share.callback_table->LoadAtomic2Return_ArrowDff)
        return modloader_reVC_share.reVC_addr_table->CFileLoader_LoadAtomicFile2Return(file);

    return modloader_reVC_share.callback_table->LoadAtomic2Return_ArrowDff(file);
}

void* ModLoader_ZonecylbDff(const char* file)
{
    if (!modloader_reVC_share.callback_table->LoadAtomic2Return_ZonecylbDff)
        return modloader_reVC_share.reVC_addr_table->CFileLoader_LoadAtomicFile2Return(file);

    return modloader_reVC_share.callback_table->LoadAtomic2Return_ZonecylbDff(file);
}

void* ModLoader_AnimFile(int32_t type, int32_t accessType, const char* pData)
{
    if (!modloader_reVC_share.callback_table->RwStreamOpen_AnimFile)
        return modloader_reVC_share.reVC_addr_table->RwStreamOpen(type, accessType, pData);

    return modloader_reVC_share.callback_table->RwStreamOpen_AnimFile(type, accessType, pData);
}

void ModLoader_InitialiseGame()
{
    if (!modloader_reVC_share.callback_table->InitialiseGame)
        return modloader_reVC_share.reVC_addr_table->InitialiseGame();

    return modloader_reVC_share.callback_table->InitialiseGame();
}


bool
ModLoader_CdStreamThread()
{
    if (modloader_reVC_share.callback_table->CdStreamThread) {
        modloader_reVC_share.callback_table->CdStreamThread();
        return true;
    }
    return false;
}


void*
ModLoader_AcquireNextModelFileHandle()
{
    if (modloader_reVC_share.callback_table->AcquireNextModelFileHandle)
        return modloader_reVC_share.callback_table->AcquireNextModelFileHandle();
    return INVALID_HANDLE_VALUE;
}

void
ModLoader_LoadCdDirectory()
{
    if (modloader_reVC_share.callback_table->LoadCdDirectory0)
        return modloader_reVC_share.callback_table->LoadCdDirectory0();
    return modloader_reVC_share.reVC_addr_table->CStreaming__LoadCdDirectory0();
}

void
ModLoader_FetchCdDirectory(const char* filename, int cd_index)
{
    if (modloader_reVC_share.callback_table->FetchCdDirectory)
        return modloader_reVC_share.callback_table->FetchCdDirectory(filename, cd_index);
    return modloader_reVC_share.reVC_addr_table->CStreaming__LoadCdDirectory2(filename, cd_index);
}

void
ModLoader_RegisterNextModelRead(int resource_id)
{
    if (modloader_reVC_share.callback_table->RegisterNextModelRead)
        return modloader_reVC_share.callback_table->RegisterNextModelRead(resource_id);
}

int32_t
ModLoader_CdStreamRead(int32_t channel, void* buffer, uint32_t offset, uint32_t size)
{
    if (modloader_reVC_share.callback_table->CdStreamThread)
        return modloader_reVC_share.callback_table->CdStreamRead(channel, buffer, offset, size);
    return modloader_reVC_share.reVC_addr_table->CdStreamRead(channel, buffer, offset, size);
}

void
ModLoader_OnRequestSpecialModel(uint32_t model_id, const char* model_name, uint32_t pos, uint32_t size)
{
    if (modloader_reVC_share.callback_table->OnRequestSpecialModel)
        return modloader_reVC_share.callback_table->OnRequestSpecialModel(model_id, model_name, pos, size);
}

const char*
ModLoader_GetCdStreamPath_Unsafe(const char* filepath)
{
    if (modloader_reVC_share.callback_table->GetCdStreamPath_Unsafe)
        return modloader_reVC_share.callback_table->GetCdStreamPath_Unsafe(filepath);
    return filepath;
}

const char*
ModLoader_GetCdDirectoryPath_Unsafe(const char* filepath)
{
    if (modloader_reVC_share.callback_table->GetCdDirectoryPath_Unsafe)
        return modloader_reVC_share.callback_table->GetCdDirectoryPath_Unsafe(filepath);
    return filepath;
}

const char*
ModLoader_RegisterAndGetColFile_Unsafe(const char* filepath)
{
    if (modloader_reVC_share.callback_table->RegisterAndGetColFile_Unsafe)
        return modloader_reVC_share.callback_table->RegisterAndGetColFile_Unsafe(filepath);
    return filepath;
}

const char*
ModLoader_RegisterAndGetAtomicFile_Unsafe(const char* filepath)
{
    if (modloader_reVC_share.callback_table->RegisterAndGetAtomicFile_Unsafe)
        return modloader_reVC_share.callback_table->RegisterAndGetAtomicFile_Unsafe(filepath);
    return filepath;
}

const char*
ModLoader_RegisterAndGetClumpFile_Unsafe(const char* filepath)
{
    if (modloader_reVC_share.callback_table->RegisterAndGetClumpFile_Unsafe)
        return modloader_reVC_share.callback_table->RegisterAndGetClumpFile_Unsafe(filepath);
    return filepath;
}

const char*
ModLoader_RegisterAndGetTexDiction_Unsafe(const char* filepath)
{
    if (modloader_reVC_share.callback_table->RegisterAndGetTexDiction_Unsafe)
        return modloader_reVC_share.callback_table->RegisterAndGetTexDiction_Unsafe(filepath);
    return filepath;
}

const char*
ModLoader_RegisterAndGetIdeFile_Unsafe(const char* filepath)
{
    if (modloader_reVC_share.callback_table->RegisterAndGetIdeFile_Unsafe)
        return modloader_reVC_share.callback_table->RegisterAndGetIdeFile_Unsafe(filepath);
    return filepath;
}

const char*
ModLoader_RegisterAndGetIplFile_Unsafe(const char* filepath)
{
    if (modloader_reVC_share.callback_table->RegisterAndGetIplFile_Unsafe)
        return modloader_reVC_share.callback_table->RegisterAndGetIplFile_Unsafe(filepath);
    return filepath;
}

const char* ModLoader_RegisterAndGetGxtFile_Unsafe(const char* filepath)
{
    if (modloader_reVC_share.callback_table->RegisterAndGetGxtFile_Unsafe)
        return modloader_reVC_share.callback_table->RegisterAndGetGxtFile_Unsafe(filepath);
    return filepath;
}

const char* ModLoader_RegisterAndGetPlayerBmpFile_Unsafe(const char* filepath)
{
    if (modloader_reVC_share.callback_table->RegisterAndGetPlayerBmpFile_Unsafe)
        return modloader_reVC_share.callback_table->RegisterAndGetPlayerBmpFile_Unsafe(filepath);
    return filepath;
}

const char* ModLoader_RegisterAndGetSplashFile_Unsafe(const char* filepath)
{
    if (modloader_reVC_share.callback_table->RegisterAndGetSplashFile_Unsafe)
        return modloader_reVC_share.callback_table->RegisterAndGetSplashFile_Unsafe(filepath);
    return filepath;
}

const char* ModLoader_GetAnimFile_Unsafe(const char* filepath)
{
    if (modloader_reVC_share.callback_table->GetAnimFile_Unsafe)
        return modloader_reVC_share.callback_table->GetAnimFile_Unsafe(filepath);
    return filepath;
}
#endif